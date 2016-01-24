SRC_PATH = .

FFMPEG_DIR=/net/public/VPM2010/Distrib/ffmpeg

AVINCLUDES = -I$(FFMPEG_DIR)/libavformat -I$(FFMPEG_DIR)/libavcodec

AVLIBS = -L$(FFMPEG_DIR)/libavformat -L$(FFMPEG_DIR)/libavcodec -L /usr/local/lib -L /usr/X11R6/lib

CXXFLAGS = -O2 $(AVINCLUDES)

LDFLAGS += $(AVLIBS) -L/usr/X11R6/lib -lGL -lGLU -lglut -ljpeg -ltiff -lpng -lCgGL -lCg -lpthread -lm -lavformat -lavcodec -lXi -lXt -lXmu

SRCS = vc-video.cpp  main.cpp  cg-lib.cpp  texjpeg.cpp  texpng.cpp  writetiff.cpp image.cpp 

HFILES = image.h vc-video.h   main.h   texjpeg.h cg-lib.h texpng.h

PROG=videoread

# storage of object files
local_libdir = obj
# storage of binary files
local_execdir = bin

SRC_FILES = $(SRCS:%=$(SRC_PATH)/%) 
OBJ_FILES = $(SRCS:%.cpp=$(local_libdir)/%.o) 

all: $(PROG)

$(PROG): $(local_execdir)/$(PROG)

$(local_libdir)/%.o : $(SRC_PATH)/%.cpp $(SRC_PATH)/$(HFILES)
	$(CXX) $(CXXFLAGS) -c -o $(local_libdir)/$*.o $*.cpp 

$(local_execdir)/$(PROG): $(OBJ_FILES)
	$(CXX) -o $@ $(OBJ_FILES) $(LDFLAGS) 

clean:
	rm -f $(OBJ_FILES) $(local_execdir)/$(PROG)
