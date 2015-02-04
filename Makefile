USE_GCC = 1
DEBUG_MODE = 1

CFLAGS =
LDFLAGS =
INCLUDES =
LIBS =

ifeq ($(USE_GCC), 1)
	include gcc.mk
else
	include icc.mk
endif

include matlab.mk

all: test_utils.$(MEXEXT)

%.$(MEXEXT): %.cpp mex_utils.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $@  $<  $(LIBS) 

clean:
	rm -rf *.o *~

distclean:	
	rm -rf *.o *~ *.$(MEXEXT)
