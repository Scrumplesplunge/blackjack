CXX = clang++

CPPFLAGS = -Dnobugs= -Dprobablynobugs=
CXXFLAGS += -std=c++17 -Wall -Wextra -Werror -pedantic
opt: CXXFLAGS += -ffunction-sections -fdata-sections -flto -Ofast -march=native
debug: CXXFLAGS += -O0 -g

LDFLAGS += -fuse-ld=gold
opt: CXXFLAGS += -DNDEBUG
opt: LDFLAGS += -s -Wl,--gc-sections -flto -Ofast

LDLIBS = -lpthread
