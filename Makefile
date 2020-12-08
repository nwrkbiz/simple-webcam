# HINT: for 3rdParty libs get https://github.com/nwrkbiz/static-build
export PATH:=3rdParty/linux_aarch64_musl/bin:3rdParty/linux_armhf_musl/bin:3rdParty/linux_x86_64_musl/bin:3rdParty/linux_i686_musl/bin:3rdParty/linux_mips_musl/bin:3rdParty/linux_mipsel_musl/bin:3rdParty/linux_ppc_musl/bin:$(PATH)
CPP=main.cpp
NAME=$(shell basename $(shell pwd))
PARAMS=-static -O3 -s -std=c++17 -lboost_system -lboost_iostreams -lboost_program_options -lssl -lcrypto  -lmgl -lmgl-fltk -lmgl -lfltk -lfltk_images  -lfreetype -lz -lpthread -latomic -ldlib -llibwebp -ltiff -lhpdfs -lgif -lturbojpeg -lopenjp2 -lpng -lgsl -llapack -lblas -lcblas -lgfortran -llapack -lblas -lm
PARAMS_LINUX=-lopencv_videoio -lopencv_imgcodecs -lopencv_calib3d -lopencv_imgproc -lopencv_core $(PARAMS) -lXinerama -lXft  -lXrender -lXfixes -lXext -lX11 -lxcb -lXau -lXdmcp -lrt -ldl
PARAMS_WINDOWS=-lopencv_videoio450 -lopencv_imgcodecs450 -lopencv_calib3d450 -lopencv_imgproc450 -lopencv_core450 -lIlmImf $(PARAMS) -DMGL_STATIC_DEFINE -DWIN32 -D_WIN32 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -mconsole -lshlwapi -lkernel32 -lole32 -loleaut32 -luser32 -luuid -lcomctl32 -lwsock32 -lws2_32 -ladvapi32 -lshell32 -lksuser -lwinmm -lcrypt32 -lgdi32

all: all_musl all_windows
all_64: linux_x86_64_musl linux_aarch64_musl windows_64
all_32: linux_i686_musl linux_armhf_musl linux_mips_musl linux_mipsel_musl linux_ppc_musl
all_musl: linux_x86_64_musl linux_i686_musl linux_armhf_musl linux_aarch64_musl linux_mips_musl linux_mipsel_musl linux_ppc_musl
all_windows: windows_32 windows_64

linux_x86_64_musl:
	x86_64-linux-musl-g++ -I3rdParty/$@/include -I3rdParty/$@/include/opencv4 -L3rdParty/$@/lib/opencv4/3rdparty -L3rdParty/$@/lib $(CPP) $(PARAMS_LINUX) -lquadmath -littnotify -o $(NAME).$@

linux_i686_musl:
	i686-linux-musl-g++ -I3rdParty/$@/include -I3rdParty/$@/include/opencv4 -L3rdParty/$@/lib/opencv4/3rdparty -L3rdParty/$@/lib $(CPP) $(PARAMS_LINUX) -lquadmath -littnotify -o $(NAME).$@

linux_armhf_musl:
	arm-linux-musleabihf-g++ -I3rdParty/$@/include -I3rdParty/$@/include/opencv4 -L3rdParty/$@/lib/opencv4/3rdparty -L3rdParty/$@/lib $(CPP) $(PARAMS_LINUX) -littnotify -o $(NAME).$@

linux_aarch64_musl:
	aarch64-linux-musl-g++ -I3rdParty/$@/include -I3rdParty/$@/include/opencv4 -L3rdParty/$@/lib/opencv4/3rdparty -L3rdParty/$@/lib $(CPP) $(PARAMS_LINUX) -ltegra_hal -littnotify -o $(NAME).$@

linux_mipsel_musl:
	mipsel-linux-musl-g++ -I3rdParty/$@/include -I3rdParty/$@/include/opencv4 -L3rdParty/$@/lib/opencv4/3rdparty -L3rdParty/$@/lib $(CPP) $(PARAMS_LINUX) -o $(NAME).$@

linux_mips_musl:
	mips-linux-musl-g++ -I3rdParty/$@/include -I3rdParty/$@/include/opencv4 -L3rdParty/$@/lib/opencv4/3rdparty -L3rdParty/$@/lib $(CPP) $(PARAMS_LINUX) -o $(NAME).$@

linux_ppc_musl:
	powerpc-linux-musl-g++ -I3rdParty/$@/include -I3rdParty/$@/include/opencv4 -L3rdParty/$@/lib/opencv4/3rdparty -L3rdParty/$@/lib $(CPP) $(PARAMS_LINUX) -o $(NAME).$@

windows_32:
	i686-w64-mingw32-windres main.32.rc mainrc.32.o
	i686-w64-mingw32-g++ -I3rdParty/$@/include -I3rdParty/$@/include/opencv4 -L3rdParty/$@/lib/opencv4/3rdparty  -L3rdParty/$@/lib  $(CPP)  mainrc.32.o  $(PARAMS_WINDOWS) -lquadmath -o $(NAME).$@.exe
	rm -rf mainrc.32.o

windows_64:
	x86_64-w64-mingw32-windres main.64.rc mainrc.64.o
	x86_64-w64-mingw32-g++ -I3rdParty/$@/include -I3rdParty/$@/include/opencv4 -L3rdParty/$@/lib/opencv4/3rdparty  -L3rdParty/$@/lib  $(CPP)   mainrc.64.o   $(PARAMS_WINDOWS) -lquadmath -o $(NAME).$@.exe
	rm -rf mainrc.64.o
clean:
	rm -rf mainrc.32.o mainrc.64.o $(NAME).*
