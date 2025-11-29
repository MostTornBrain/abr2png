# --- Configuration Variables ---
# Use different variables for Linux (CC) and Windows (CC_WIN) compilers
CC         = g++
CC_WIN     = x86_64-w64-mingw32-g++

CFLAGS     = -Wall -pedantic
CFLAGS_OPT = -O2

# Note: Windows doesn't typically need -lstdc++ explicitly, 
# but we need to ensure we link the correct Windows versions of libpng and zlib.
LIBS       = -lpng -lz

# Source files list
SRCS       = abr_util.cpp PngWrite.cpp abr.cpp
OBJS       = $(SRCS:.cpp=.o)
OBJS_WIN   = $(SRCS:.cpp=.o_win) # Use different object file names for Windows build to prevent conflicts

TARGET_LINUX = abr2png
TARGET_WIN   = abr2png.exe


# --- Build Targets ---

.PHONY: all linux windows clean

# Default build (Linux)
all linux: $(TARGET_LINUX)

# Windows build target
windows: $(TARGET_WIN)

# --- Compilation Rules ---

# Rule to link the Linux executable
$(TARGET_LINUX): $(OBJS)
	$(CC) $(CFLAGS_OPT) $^ -o $@ $(LIBS)
	@echo "--- Linux build complete: $(TARGET_LINUX) ---"

# Rule to link the Windows executable
$(TARGET_WIN): $(OBJS_WIN)
	$(CC_WIN) $(CFLAGS_OPT) $^ -o $@ $(LIBS)
	@echo "--- Windows build complete: $(TARGET_WIN) ---"

# Generic rule for Linux object files (.o)
%.o: %.cpp
	$(CC) $(CFLAGS) $(CFLAGS_OPT) -c $< -o $@

# Generic rule for Windows object files (.o_win)
%.o_win: %.cpp
	$(CC_WIN) $(CFLAGS) $(CFLAGS_OPT) -c $< -o $@

# Clean up all created binaries and object files
clean:
	rm -rf $(TARGET_LINUX) $(TARGET_WIN) $(OBJS) $(OBJS_WIN)
	@echo "Cleaned up build artifacts."

