MATLABDIR = /usr/local/matlabR2010a

MATLABARCH = glnxa64
MEXEXT = $(shell $(MATLABDIR)/bin/mexext)
MAPFILE = mexFunction.map

MATLABLIBS = -L$(MATLABDIR)/bin/$(MATLABARCH) -lmx -lmex -lmat
RPATH = -Wl,-rpath-link,$(MATLABDIR)/bin/$(MATLABARCH)
LIBS = $(RPATH) $(MATLABLIBS) -lm -lpthread 

MATLABINCLUDE = -I$(MATLABDIR)/extern/include
INCLUDES = $(MATLABINCLUDE)

CC = g++
MEXFLAGS = -DUSE_MATLAB_INTERFACE -DMATLAB_MEX_FILE -D_GNU_SOURCE -fexceptions -fno-omit-frame-pointer
GENERALFLAGS = -fPIC -W -Wall -Wextra -g -pedantic 
OPTIMFLAGS = -march=native -O3 -ffast-math -fopenmp -pthread
REPORTSFLAGS = -Winline -Wimplicit
DEBUGFLAG = -g
ifdef DEBUG_MODE
	CFLAGS = $(DEBUGFLAG) $(MEXFLAGS) $(GENERALFLAGS) 
else
	CFLAGS = $(MEXFLAGS) $(GENERALFLAGS) $(OPTIMFLAGS)
	ifdef PRODUCE_REPORTS
		CFLAGS += $(REPORTSFLAGS) 
	endif
endif 

LDFLAGS = -pthread -shared -Wl,--version-script,$(MATLABDIR)/extern/lib/$(MATLABARCH)/$(MAPFILE) -Wl,--no-undefined
