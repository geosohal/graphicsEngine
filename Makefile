########################################################################
# Makefile for Linux

ifeq (sol,$(flag))
    SFLAG := -DSOLUTION
    ODIR := sobjs
else
    SFLAG := 
    ODIR := objs
endif

vpath %.o  $(ODIR)

ifneq (,$(wildcard libs))
    LIBDIR := libs
else
    ifneq (,$(wildcard ../libs))
        LIBDIR := ../libs
    else
        ifneq (,$(wildcard ../../libs))
            LIBDIR := ../../libs
        else
            LIBDIR := ../../../libs
        endif
    endif
endif

CXX = g++
CFLAGS = -g $(SFLAG) -I. -I$(LIBDIR)/glbinding  -I$(LIBDIR)/freeglut -I$(LIBDIR)/glm -I$(LIBDIR) -I/usr/include -I/usr/include/GL/ 
CXXFLAGS = -std=c++11 $(CFLAGS)

LIBS =  -L/usr/lib/x86_64-linux-gnu -L../$(LIBDIR) -L/usr/lib -L/usr/local/lib -lglbinding -lglut -lX11 -lGLU -lGL

CPPsrc = framework.cpp interact.cpp transform.cpp scene.cpp texture.cpp shapes.cpp object.cpp shader.cpp simplexnoise.cpp fbo.cpp
Csrc = rply.c

headers = framework.h interact.h texture.h shapes.h object.h rply.h scene.h shader.h transform.h simplexnoise.h fbo.h
shaders = lighting.vert lighting.frag 
srcFiles = $(CPPsrc) $(Csrc) $(shaders) $(headers)
extraFiles = framework.vcxproj Makefile box.ply textures

pkgName = $(notdir CS541-framework)
solName = $(notdir CS562-framework)
objs = $(patsubst %.cpp,%.o,$(CPPsrc)) $(patsubst %.c,%.o,$(Csrc))
target = $(ODIR)/framework.exe

$(target): $(objs)
	@echo Link $(target)
	cd $(ODIR) && $(CXX) -g  -o ../$@  $(objs) $(LIBS)

run: $(target)
	LD_LIBRARY_PATH="$(LIBDIR);$(LD_LIBRARY_PATH)" ./$(target)

what:
	@echo VPATH = $(VPATH)
	@echo LIBS = $(LIBDIR)
	@echo SFLAG = $(SFLAG)
	@echo objs = $(objs)

clean:
	rm -rf objs sobjs dependencies

fill:
	cp -n ../../$(pkgName)/* .

%.o: %.cpp
	@echo Compile $<  $(SFLAG)
	@mkdir -p $(ODIR)
	@$(CXX) -c $(CXXFLAGS) $< -o $(ODIR)/$@

%.o: %.c
	@echo Compile $< $(SFLAG)
	@mkdir -p $(ODIR)
	@$(CC) -c $(CFLAGS) $< -o $(ODIR)/$@

zip: $(srcFiles) $(extraFiles)
	rm -rf ../$(pkgName) ../$(pkgName).zip
	mkdir ../$(pkgName)
	../SolutionFilter.py $(srcFiles) ../$(pkgName)
	cp -r $(extraFiles) ../$(pkgName)
	cp -r ../libs ../$(pkgName)
	rm -rf ../$(pkgName)/libs/.hg ../$(pkgName)/libs/Eigen* ../$(pkgName)/libs/assimp
	cd ..;  zip -r $(pkgName).zip $(pkgName); rm -rf $(pkgName)

solzip: $(srcFiles) $(extraFiles)
	rm -rf ../$(solName) ../$(solName).zip
	mkdir ../$(solName)
	cp $(srcFiles) ../$(solName)
	cp -r $(extraFiles) ../$(solName)
	cp -r ../libs ../$(solName)
	rm -rf ../$(solName)/libs/.hg ../$(solName)/libs/Eigen* ../$(solName)/libs/assimp
	cd ..;  zip -r $(solName).zip $(solName); rm -rf $(solName)

ws:
	unix2dos $(srcFiles)
	@echo
	@echo ========= TABS:
	@grep -P '\t' $(srcFiles)

dependencies: 
	g++ -MM $(CXXFLAGS) $(CPPsrc) > dependencies

include dependencies
