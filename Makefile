BIN = syspower
LIB = libsyspower.so
PKGS = gtkmm-4.0 gtk4-layer-shell-0	
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
DATADIR ?= $(PREFIX)/share

CXXFLAGS += -Oz -s -Wall -flto -fno-exceptions -fPIC
LDFLAGS += -Wl,--as-needed,-z,now,-z,pack-relative-relocs

CXXFLAGS += $(shell pkg-config --cflags $(PKGS))
LDFLAGS += $(shell pkg-config --libs $(PKGS))

JOB_COUNT := $(BIN) $(LIB) $(OBJS) src/git_info.hpp
JOBS_DONE := $(shell ls -l $(JOB_COUNT) 2> /dev/null | wc -l)

define progress
	$(eval JOBS_DONE := $(shell echo $$(($(JOBS_DONE) + 1))))
	@printf "[$(JOBS_DONE)/$(shell echo $(JOB_COUNT) | wc -w)] %s %s\n" $(1) $(2)
endef

all: $(BIN) $(LIB)

install: $(all)
	@echo "Installing..."
	@install -D -t $(DESTDIR)$(BINDIR) $(BIN)
	@install -D -t $(DESTDIR)$(LIBDIR) $(LIB)
	@install -D -t $(DESTDIR)$(DATADIR)/sys64/power config.conf style.css

clean:
	@echo "Cleaning up"
	@rm $(BIN) $(LIB) $(OBJS) src/git_info.hpp

$(BIN): src/git_info.hpp src/main.o src/config_parser.o
	$(call progress, Linking $@)
	@$(CXX) -o $(BIN) \
	src/main.o \
	src/config_parser.o \
	$(CXXFLAGS) \
	$(shell pkg-config --libs gtkmm-4.0 gtk4-layer-shell-0)

$(LIB): $(OBJS)
	$(call progress, Linking $@)
	@$(CXX) -o $(LIB) \
	$(filter-out src/main.o src/config_parser.o, $(OBJS)) \
	$(CXXFLAGS) \
	$(LDFLAGS) \
	-shared

%.o: %.cpp
	$(call progress, Compiling $@)
	@$(CXX) $(CFLAGS) -c $< -o $@ \
	$(CXXFLAGS)

src/git_info.hpp:
	$(call progress, Creating $@)
	@commit_hash=$$(git rev-parse HEAD); \
	commit_date=$$(git show -s --format=%cd --date=short $$commit_hash); \
	commit_message=$$(git show -s --format="%s" $$commit_hash | sed 's/"/\\\"/g'); \
	echo "#define GIT_COMMIT_MESSAGE \"$$commit_message\"" > src/git_info.hpp; \
	echo "#define GIT_COMMIT_DATE \"$$commit_date\"" >> src/git_info.hpp
