LIB=		libgenloc.a
INCLUDE = location.h
DLIB=libgenloc$(DSUFFIX)
LICENSES= license_libgenloc.txt

MAN3 = genloc.3 \
       genloc_intro.3 \
       ggnloc.3 \
       eqerror.3 \
	genloc_ttinterface.3

DATADIR=schemas
DATA=genloc1.0

CLEAN = $(LICENSES)

SUBDIR=/contrib
include $(ANTELOPEMAKE)  	
DIRS=	

OBJS=	  bisquare.o \
    calc_statistics.o \
    dbinput_routines.o \
    distance_weight.o \
    erfinv.o \
    eqerror.o \
    generic_tu_calculate.o \
    huber.o \
    initial_locate.o \
    input_routines.o \
    locate.o \
    matrix.o \
    save_predarr.o \
    svdcmp.o \
    table_interpolate_functions.o \
    thomson.o \
    ttcalc_interface.o\
    ttlvz.o \
    ttlvz_interface.o \
    SminusP.o

$(LIB) : $(OBJS)
	$(AR) $(ARFLAGS) $@ $(LORDER) $(OBJS) $(TSORT) 
	$(RANLIB) $@

$(DLIB) : $(OBJS)
	case `uname -s` in \
	Darwin) $(DLD) $(CONTRIBDLDFLAGS) -o $@ $(LORDER) $(OBJS) $(TSORT) $(DLDLIBS) $(ANTELOPE)/contrib/static/libperf.a ;; \
	*) $(DLD) $(CONTRIBDLDFLAGS) -o $@ $(LORDER) $(OBJS) $(TSORT) $(DLDLIBS)  ;; \
	esac 

$(LICENSES) : LICENSE
	cp $< $@

# $Id$ 
