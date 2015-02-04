MATLABDIR = /usr/local/matlab
MATLABARCH = glnxa64
MEXEXT = $(shell $(MATLABDIR)/bin/mexext)
MAPFILE = mexFunction.map

MATLABLIBS = -L$(MATLABDIR)/bin/$(MATLABARCH) -lmx -lmex -lmat
RPATH = -Wl,-rpath-link,$(MATLABDIR)/bin/$(MATLABARCH)
LIBS += $(RPATH) $(MATLABLIBS)

MATLABINCLUDE= -I$(MATLABDIR)/extern/include
INCLUDES += $(MATLABINCLUDE)

MEXFLAGS = -DUSE_MATLAB_INTERFACE -DMATLAB_MEX_FILE -D_GNU_SOURCE -fexceptions -fno-omit-frame-pointer
CFLAGS += $(MEXFLAGS)
LDFLAGS += -pthread -shared -Wl,--version-script,$(MATLABDIR)/extern/lib/$(MATLABARCH)/$(MAPFILE) -Wl,--no-undefined
