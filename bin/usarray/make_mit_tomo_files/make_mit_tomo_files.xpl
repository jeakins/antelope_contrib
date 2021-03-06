#
#   program needs:
#
#
    use POSIX ;    
    use strict ;
    use Datascope ;
    use archive;
    use timeslice ;
    use utilfunct ;
    use utility ;
    use Getopt::Std ;
    
    our ( $pgm, $host );
    our ( $opt_V, $opt_e, $opt_t, $opt_v );
    
{    #  Main program

    my ( $atime, $cmd, $db, $delta, $depth, $elev, $esaz, $event_file, $lat, $lon ) ;
    my ( $mb, $ms, $norigin, $nse, $orid, $otime, $phase_file, $pslow, $ptt, $radius ) ;
    my ( $slat, $slon, $stime, $string, $subject, $table, $time, $tres, $tt, $usage ) ;
    my ( @db, @dbarrival, @dbassoc, @dbevent, @dborigin, @dbse, @dbsite, @dbt ) ;

    $pgm = $0 ; 
    $pgm =~ s".*/"" ;
    elog_init( $pgm, @ARGV ) ;
    $cmd = "\n$0 @ARGV" ;
    
    if (  ! &getopts('e:pt:vV') || ( @ARGV != 1 ) ) { 
        $usage  =  "\n\n\nUsage: $0  dbtomo"  ;         
        elog_notify($cmd) ; 
        elog_die ( $usage ) ; 
    }
 
    $opt_v      = defined($opt_V) ? $opt_V : $opt_v ;    
    chop ($host = `uname -n` ) ;
    
    announce( 0, 0 ) ;
    elog_notify( $cmd ) ; 
    $stime = strydtime( now() ) ;
    elog_notify( "\nstarting execution on	$host	$stime\n\n" ) ;
    
    $db        = $ARGV[0] ;

    @db        = dbopen   ( $db, "r" ) ;
    @dbevent   = dblookup ( @db, 0, "event", 0, 0) ;
    @dborigin  = dblookup ( @db, 0, "origin", 0, 0) ;
    @dborigin  = dbsort   ( @dborigin, "time" ) ;
    @dbassoc   = dblookup ( @db, 0, "assoc", 0, 0) ;
    @dbarrival = dblookup ( @db, 0, "arrival", 0, 0) ;
    @dbsite    = dblookup ( @db, 0, "site", 0, 0) ;
    
    foreach $table ( qw ( event origin assoc arrival site ) ) {
        @dbt = dblookup ( @db, 0, $table, 0, 0) ;
        elog_die ( "$table does not exist in $db!" ) if ( ! dbquery ( @dbt, "dbTABLE_PRESENT" ) ) ;
    }
    
    @dbse     = dbjoin   ( @dborigin, @dbassoc ) ;
    @dbse     = dbjoin   ( @dbse, @dbarrival ) ;
    @dbse     = dbjoin   ( @dbse, @dbsite ) ;
    @dbse     = dbsubset ( @dbse, "iphase =~ /P/ " ) ;
    @dbse     = dbsubset ( @dbse, "phase =~ /P/ || delta < 75. " ) ;

    
    $norigin  = dbquery ( @dborigin, "dbRECORD_COUNT" ) ;
    $nse      = dbquery ( @dbse, "dbRECORD_COUNT" ) ;
    
    elog_notify ( "norigin  $norigin" ) ;
    elog_notify ( "nse      $nse" ) ;
    
    $event_file = "ta_P_events_" . yearday( dbex_eval ( @dborigin, "min(time) " ) ) . "_" . yearday( dbex_eval ( @dborigin, "max(time) " ) ) ;
    $phase_file = "ta_P_phase_" . yearday( dbex_eval ( @dborigin, "min(time) " ) ) . "_" . yearday( dbex_eval ( @dborigin, "max(time) " ) ) ;
    
    elog_notify ( "opening	$event_file" ) ;
    
    open( EV, ">$event_file" );
   
    for ( $dborigin[3] = 0 ; $dborigin[3] < $norigin ; $dborigin[3]++ ) {
        ( $orid, $time, $lat, $lon, $depth, $mb, $ms ) = dbgetv ( @dborigin, qw ( orid time lat lon depth mb ms ) ) ;
        $lat    = rad ( $lat ) ;
        if ( $lon < 0.0 ) {
            $lon = 360. + $lon ;
        } 
        $lon    = rad ( $lon ) ;

        
        $radius = 6371 - $depth ;
        
        $string = sprintf ( "%9d  %22s  %12.9f  %12.9f  %7.2f  %6.1f  %6.1f ", $orid, epoch2str ( $time, "%m %d %Y %H %M %S.%s"), $lat, $lon, $radius, $mb, $ms ) ;
        print EV "$string \n" ;
    }
    
    close ( EV );
   
    elog_notify ( "opening	$phase_file" ) ;
    
    open( PH, ">$phase_file" );
   
    for ( $dbse[3] = 0 ; $dbse[3] < $nse ; $dbse[3]++ ) {
        ( $orid, $lat, $lon, $depth, $slat, $slon, $elev, $otime, $atime, $delta, $esaz, $tres ) = dbgetv ( @dbse, qw ( orid origin.lat origin.lon depth site.lat site.lon elev origin.time arrival.time delta esaz timeres ) ) ;

        $lat    = rad ( $lat ) ;
        if ( $lon < 0.0 ) {
            $lon = 360. + $lon ;
        } 
        $lon    = rad ( $lon ) ;
        
        $radius = 6371 - $depth ;
        
        $slat    = rad ( $slat ) ;
        if ( $slon < 0.0 ) {
            $slon = 360. + $slon ;
        } 
        $slon    = rad ( $slon ) ;
        
        if ( $esaz < 0.0 ) {
            $esaz = 360. + $esaz ;
        } 
        $esaz    = rad ( $esaz ) ;
        
        $tt      = $atime - $otime ;
        
        $ptt     = dbex_eval ( @dbse, "ptime( $delta, $depth )" ) ;

        $pslow   = dbex_eval ( @dbse, "pphase_slowness( $delta, $depth )" ) ;

        $string = sprintf ( "%12.9f  %12.9f  %7.2f  %9d  %12.9f  %12.9f  %7.2f  %5.2f  %7.2f  %7.2f  %7.2f  %8.4f  %8.4f", 
                             $lat,   $lon, $radius, $orid, $slat, $slon, $elev, $tres, $ptt,  $tt,  $delta, $esaz, $pslow  ) ;
        print PH "$string \n" ;
    }
     
    close ( PH );
    
    $stime = strydtime(now());
    elog_notify ("completed successfully	$stime\n\n");

    $subject = sprintf("Success  $pgm  $host");
    elog_notify ($subject);
    
    exit(0) ;
    
}

