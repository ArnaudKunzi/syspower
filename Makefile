EXEC = syspower
LIB = libsyspower.so
PKGS = gtkmm-4.0 gtk4-layer-shell-0	
SRCS = $(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJS = $(SRCS:.cpp=.o)
DESTDIR = $(HOME)/.local

CXXFLAGS = -march=native -mtune=native -Os -s -Wall -flto=auto -fno-exceptions -fPIC
CXXFLAGS += $(shell pkg-config --cflags $(PKGS))
LDFLAGS = $(shell pkg-config --libs $(PKGS))

all: $(EXEC) $(LIB)

install: $(all)
	mkdir -p $(DESTDIR)/bin $(DESTDIR)/lib
	install $(EXEC) $(DESTDIR)/bin/$(EXEC)
	install $(LIB) $(DESTDIR)/lib/$(LIB)

clean:
	rm $(EXEC) $(LIB) $(SRCS:.cpp=.o) src/git_info.hpp

$(EXEC): src/git_info.hpp
	$(CXX) -o $(EXEC) \
	src/main.cpp \
	$(CXXFLAGS) \
	$(LDFLAGS)

$(LIB): $(OBJS)
	$(CXX) -o $(LIB) \
	$(OBJS) \
	$(CXXFLAGS) \
	-shared

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ \
	$(CXXFLAGS)

src/git_info.hpp:
	@commit_hash=$$(git rev-parse HEAD); \
	commit_date=$$(git show -s --format=%cd --date=short $$commit_hash); \
	commit_message=$$(git show -s --format=%s $$commit_hash); \
	echo "#define GIT_COMMIT_MESSAGE \"$$commit_message\"" > src/git_info.hpp; \
	echo "#define GIT_COMMIT_DATE \"$$commit_date\"" >> src/git_info.hpp
