from twisted.python import log 
from twisted.internet import reactor
import sys
import os

from collections import defaultdict 

from antelope.datascope import *
from antelope.stock import *

import dbwfserver.config as config
    
class EventData():

    """
    Provide interaction with Datascope database
    """

    def __init__(self, dbname):

        self.dbname = dbname
        self.db = dbopen(self.dbname)
        self.stachan_cache = defaultdict(lambda: defaultdict(dict))
        self.event_cache = defaultdict(lambda: defaultdict(dict))

    def _get_stachan_cache(self):

        self.stachan_cache = defaultdict(lambda: defaultdict(dict))

        db = Dbptr(self.db)
        db.process([
            'dbopen sitechan',
            'dbjoin sensor',
            'dbjoin instrument'
            ])

        for i in range(db.query(dbRECORD_COUNT)):

            db.record = i

            sta, chan, insname, srate, ncalib, rsptype = db.getv('sta', 'chan', 'insname', 'samprate', 'ncalib','rsptype')

            if config.debug:
                log.msg("\tStation: %s %s %s %s %s %s" % (sta,chan,insname,srate,ncalib,rsptype))

            self.stachan_cache[sta][chan]['insname'] = insname
            self.stachan_cache[sta][chan]['samprate'] = srate
            self.stachan_cache[sta][chan]['ncalib'] = ncalib
            self.stachan_cache[sta][chan]['rsptype'] = rsptype

        self.call = reactor.callLater(600, self._get_stachan_cache)

    def _get_event_cache(self):

        self.event_cache = defaultdict(lambda: defaultdict(dict))

        db = Dbptr(self.db)

        db.lookup( table='event')

        if db.query(dbTABLE_PRESENT):

            if config.debug:
                log.msg("Event table present!")

            db.process([
                'dbopen event',
                'dbjoin origin',
                'dbsubset orid == prefor',
                'dbjoin assoc',
                'dbsort -u sta orid'
            ])

        else:

            if config.debug:
                log.msg("Event table NOT present")

            db.process([
                'dbopen origin',
                'dbjoin assoc',
                'dbsort -u sta orid'
            ])

        for i in range(db.query(dbRECORD_COUNT)):

            db.record = i

            sta,orid = db.getv('sta', 'orid')

            if config.debug:
                log.msg("\tEvent:%s %s" % (sta,orid))

            try: self.event_cache[sta].append(orid)
            except: self.event_cache[sta] = [orid]

        self.call = reactor.callLater(600, self._get_event_cache)



    def _get_orid_data(self, origin, stations=None):
        """
        Go through station groups to retrieve all events
        with arrival times for different phases
        """
        db = Dbptr(self.db)

        db.process([
            'dbopen origin',
            'dbjoin assoc',
            'dbjoin arrival'
        ])
        origin_sub = dbsubset(db,'orid == %d' % (origin) ) # Subset on origin

        if not origin_sub.query(dbRECORD_COUNT) > 0:
            log.msg('\n')
            log.msg("Error in orid:%s . NOT IN DATABASE!" % origin)
            log.msg('\n')
            return False


        origin_sub.sort('phase')

        origin = defaultdict(lambda: defaultdict(dict))

        origin = { 
            'origin_time':'', 
            'lat':'', 
            'lon':'', 
            'depth':'', 
            'magnitude':'', 
            'mtype':'', 
            'auth':'', 
            'review':'',
        }

        origin_sub.record = 0

        time, lat, lon, depth, auth, review, mb, ms, ml = origin_sub.getv('time', 'lat', 'lon', 'depth', 'auth', 'review', 'mb', 'ms', 'ml')

        if config.debug:
            log.msg("\tEvent: %s %s %s %s %s %s %s %s %s" % (time,lat,lon,depth,auth,review,mb,ms,ml))

        origin['origin_time'] = time
        origin['readable_time'] = epoch2str(time,"%Y-%m-%d %H:%M:%S UTC");
        origin['lat'] = lat
        origin['lon'] = lon
        origin['depth'] = depth
        origin['auth'] = auth
        origin['review'] = review

        if mb > 0:
            origin['magnitude'] = mb
            origin['mtype'] = 'Mb'
        elif ms > 0:
            origin['magnitude'] = ms
            origin['mtype'] = 'Ms'
        elif ml > 0:
            origin['magnitude'] = ml
            origin['mtype'] = 'Ml'


        origin['phases'] = defaultdict(lambda: defaultdict(dict))

        for k in range(origin_sub.query(dbRECORD_COUNT)):

            origin_sub.record = k

            time, iphase, phase, station, chan = origin_sub.getv('arrival.time', 'iphase', 'phase', 'sta', 'chan')
            if config.debug:
                log.msg("\tEvent: %s %s %s %s %s" % (time,iphase,phase,station,chan))

            if stations:
                for sta in stations:
                    if sta == station:
                        origin['phases'][station][chan]['arrival_time'] = time
                        origin['phases'][station][chan]['iphase'] = iphase
                        origin['phases'][station][chan]['phase'] = phase
                        
            else:
                origin['phases'][station][chan]['arrival_time'] = time
                origin['phases'][station][chan]['iphase'] = iphase
                origin['phases'][station][chan]['phase'] = phase

        return origin


    def available_stations(self,sta=None):

        """
        Get a list of stations that have data in the database
        by just returning keys from _get_stachan_cache() query
        """
        temp_dic = {}

        if not self.stachan_cache:
            self._get_stachan_cache()

        if  not self.stachan_cache.keys():
            temp_dic['error'] = ("No stations out of DB query. Need a join of SITECHAN SENSOR and INSTRUMENT tables." )
            log.msg("Exceptionon data: %s" % temp_dic['error'])
            return temp_dic
                

        if not sta:
            return self.stachan_cache.keys()
        else:
            for st in sta:
                temp_dic[st] = self.stachan_cache[st]
            return temp_dic

    def event_list(self, sta=None, orid=None):

        """
        Get a list of events from the database
        by just returning keys from _get_events_cache() query
        Test with:
        http://localhost:8008/data?type=events
        or 
        http://localhost:8008/data?type=events&sta=127A - list of events recorded by station 127A
        or 
        http://localhost:8008/data?type=events&orid=66484 - list of stations that recorded event 66484
        or 
        http://localhost:8008/data?type=events&sta=127A&orid=66484 - returns a floating point that is the arrival time
        """

        if not self.event_cache:
            self._get_event_cache()

        list = []   # List of orids or stations
        temp_dic = {}

        log.msg("Event_list function. STA=%s ORID=%s" % (sta,orid) )

        if  not self.event_cache.keys():
            temp_dic['error'] = ("No events out of DB query. Need a join of [EVENT] ORIGIN and ASSOC tables." )
            log.msg("Exceptionon data: %s" % temp_dic['error'])
            return temp_dic

        if sta and orid:

            return self._get_orid_data(orid,sta)

        elif sta and not orid:

            for st in sta:
                temp_dic[st] = self.event_cache[st]
            return temp_dic

        elif orid and not sta:

            return self._get_orid_data(orid)

        else:

            return self.event_cache


    def get_segment(self, sta, chan, canvas_size, orid=None, time_window=None, mintime=None, maxtime=None, filter=None):

        """
        Get a segment of waveform data.
    
        Return a list of (time, value) or (time, min, max) tuples,
        e.g: [(t1, v1), (t2, v2), ...]
        or
             [(t1, v1min, v1max), (t2, v2min, v2max), ...]

        TEST:
            http://localhost:8008/data?type=wf&sta=113A&orid=66554
    
        Client-side plotting library, Flot, plots the following 
        [time,max,min] - hence need to rearrange
        from [time,min,max] to [time,max,min]
        Javascript takes milliseconds, so multiply utc time
        by 1000

        Also return event metadata
        """
        if not self.stachan_cache:
            self._get_stachan_cache()

        res_data = defaultdict(lambda: defaultdict(dict))

        db = Dbptr(self.db)
        db.lookup(table="wfdisc")

        if config.debug:
            log.msg("Starting functions eventdata.get_segment")
            log.msg("\tsta:\t%s"        % sta)
            log.msg("\tchan:\t%s"       % chan)
            log.msg("\torid:\t%s"       % orid)
            log.msg("\ttime_window:\t%s"% time_window)
            log.msg("\tmintime:\t%s"    % mintime)
            log.msg("\tmaxtime:\t%s"    % maxtime)
            log.msg("\tcanvas:\t%s"     % canvas_size)
            log.msg("\tfilter:\t%s"     % filter)

        if orid:
            resp_data = {'metadata':self._get_orid_data(orid)}
            if not resp_data['metadata']['origin_time']:
                log.msg("No origin time for this event:%s" % orid)
                return False

            orid_time = resp_data['metadata']['origin_time']
            if config.verbose: log.msg( 'Looking for origin time: %s' % orid_time)

            if not mintime or not maxtime:

                if not time_window: time_window = config.default_time_window

                maxtime =  orid_time + ( time_window/2 )
                mintime =  orid_time - ( time_window/2 )

        else:
            if config.verbose: log.msg( 'No orid passed - ignore metadata' )

        # Use either passed min and maxtimes, or origin_time generated equivalents

        if not maxtime or maxtime == -1 or mintime == -1 or not mintime:
            log.msg("Error in maxtime:%s or mintime:%s" % (maxtime,mintime))
            return  

        res_data.update( {'type':'waveform'} )
        res_data.update( {'time_start':mintime} )
        res_data.update( {'time_end':maxtime} )
        res_data.update( {'time_window':time_window} )
        res_data.update( {'orid':orid} )
        res_data.update( {'sta':sta} )
        res_data.update( {'chan':chan} )

        for station in sta:
            for channel in chan:
                if config.debug: log.msg("Now: %s %s" % (station,channel))
                if not self.stachan_cache[station][channel].get('samprate',False):
                    log.msg('\n')
                    log.msg('ERROR: %s %s not a valid station and channel combination!' % (station,channel))
                    log.msg('\n')
                    continue

                temp = []

                if config.verbose: log.msg("Log times: %s %s" % (mintime,maxtime))

                res_data[station][channel].update({'start':mintime})
                res_data[station][channel].update({'end':maxtime})
                res_data[station][channel].update({'metadata':self.stachan_cache[station][channel]})

                if config.debug: log.msg("trloadchan(%s %s %s %s)" % (mintime,maxtime,station,channel))

                try:
                    tr = db.loadchan(mintime,maxtime,station,channel)
                except Exception,e:
                    res_data['error'] = ("%s" % e)
                    log.msg("Exceptionon trloadchan: %s" % e)

                tr.record = 0

                log.msg("samprate: %s" % self.stachan_cache[station][channel]['samprate'])

                points = int( (maxtime-mintime)*self.stachan_cache[station][channel]['samprate'])

                log.msg("Total points:%s Canvas Size:%s Binning threshold:%s" % (points,canvas_size,config.binning_threshold))

                if not points > 0:
                    res_data[station][channel]['data'] = ()

                elif points <  (config.binning_threshold * canvas_size):

                    if filter:
                        log.msg("Filter is: %s" % (filter))
                        tr.filter(filter)

                    try:
                        res_data[station][channel]['data'] = tr.data()
                    except Exception,e:
                        res_data['error'] = ("%s" % e)
                        log.msg("Exceptionon data: %s" % e)

                    res_data[station][channel]['format'] = 'lines'

                else:

                    try:
                        res_data[station][channel]['data'] = tr.databins(points/canvas_size)
                    except Exception,e:
                        res_data['error'] = ("%s" % e)
                        log.msg("Exceptionon databins: %s" % e)

                    res_data[station][channel]['format'] = 'bins'


                trfree(tr)
                trdestroy(tr)

        return res_data


    def coverage(self, sta=None, chan=None, start=None, end=None):

        """
        Get list of segments of data for the respective station and channel

        Return a list of (start, end) tuples,
        e.g: [(s1, e1), (s2, e2), ...]

        TEST:
            http://localhost:8008/data?type=coverage&sta=113A&chan=BHZ

        """
        sta_string = ''
        chan_string = ''
        response_data = defaultdict(lambda: defaultdict(dict))

        if config.verbose:
            log.msg("Getting segment for:")
            log.msg("\tsta:\t%s"        % sta)
            log.msg("\tchan:\t%s"       % chan)
            log.msg("\tstart:\t%s"      % start)
            log.msg("\tmintime:\t%s"    % end)

        if not sta:
            sta = self.available_stations()

        if not chan:
            chan = config.default_chans

        for station in sta:
            if sta_string: sta_string = sta_string + '|'
            sta_string = sta_string + station

        for channel in chan:
            if chan_string: chan_string = chan_string + '|'
            chan_string = chan_string + channel

        db = Dbptr(self.db)
        db.lookup(table="wfdisc")

        if config.verbose: log.msg("\tdbsubset: sta =~/%s/ && chan =~/%s/" % (sta_string, chan_string))
        db.subset("sta =~/%s/ && chan =~/%s/" % (sta_string, chan_string))

        if start:
            if config.verbose: log.msg("\tdbsubset: endtime >= %s" % start)
            db.subset("endtime >= %s" % start)

        if end:
            if config.verbose: log.msg("\tdbsubset: time <= %s" % end)
            db.subset("time <= %s" % end)

        if not db.query(dbRECORD_COUNT):
            log.msg("\tRecords on DB:\t%s" % db.query(dbRECORD_COUNT))
            log.msg('No records on subset for sta:%s chan:%s st:%s et:%s' % (sta_string,chan_string,start,end))
            response_data.update( {'time_start':0} )
            response_data.update( {'time_end':0} )
            response_data.update( {'sta':sta} )
            response_data.update( {'chan':chan} )
            return response_data


        tmin = db.ex_eval('min(time)')
        tmax = db.ex_eval('max(endtime)')

        if start:
            tmin = start
        if end:
            tmax = end

        response_data.update( {'time_start':tmin} )

        response_data.update( {'time_end':tmax} )

        response_data.update( {'sta':sta} )

        response_data.update( {'chan':chan} )

        for i in range(db.query(dbRECORD_COUNT)):

            db.record = i

            (st, ch, time, endtime) = db.getv('sta', 'chan', 'time', 'endtime')

            if config.verbose: log.msg("\tGot: %s %s %s %s" % (st,ch,time,endtime))

            if start and start > time:
                time = start
            
            if end and end < endtime:
                endtime = end

            if config.verbose: log.msg("\tresponse_data[%s][%s].update(%s,%s)" % (st,ch,time,endtime))

            response_data[st][ch].update( {time:endtime} )


        return response_data

