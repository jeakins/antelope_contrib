import antelope.datascope as datascope
import _polygon
def inwhichpolygons(db, lat,lon):
    """find enclosing polygon"""
    # Ugly but clear: Kent says, for C, a database is only a list
    mydb=[db.database, db.table,db.field, db.record]
    mydbr = _polygon._inwhichpolygons( mydb, lat, lon )
    #and therefore we only get a list from C, and have to create a Dbptr object based on that
    # again very ugly, but totally clear...
    retval=datascope.Dbptr( (mydbr[0], mydbr[1], mydbr[2], mydbr[3]) )   
    return retval

def distancetopolygon(db, lat,lon):
    """find enclosing polygon"""
    # Ugly but clear: Kent says, for C, a database is only a list
    mydb=[db.database, db.table,db.field, db.record]
    retval = _polygon._distancetopolygon( mydb, lat, lon )
    return retval

def windrose(azimuth):
    """windrose"""
    retval = _polygon._windrose(azimuth )
    return retval

