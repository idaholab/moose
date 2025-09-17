CXXFLAGS += -g -Wall -Wextra -pthread
STD := -std=c++17

# ---- HIT files ---------------------------------------------------------
# Compiler order of priority: libmesh_CXX, CXX, g++
hit_CXX = $(libmesh_CXX)
ifeq ($(hit_CXX),)
  hit_CXX := $(CXX)
endif
ifeq ($(hit_CXX),)
  hit_CXX := g++
endif

hit_srcdir         := ../src/hit
hit_srcfiles       := $(hit_srcdir)/parse.cc $(hit_srcdir)/lex.cc $(hit_srcdir)/braceexpr.cc
hit_includeflags   := -I../include

# ---- WASP files --------------------------------------------------------

WASP_DIR           ?= $(abspath ../../wasp/install)

# find the wasp libraries
lib_suffix := so
ifeq ($(shell uname -s),Darwin)
	lib_suffix     := dylib
	wasp_LIBS      := $(shell find -E $(WASP_DIR)/lib -regex ".*/lib[a-z]+.$(lib_suffix)")
else
	wasp_LIBS      := $(wildcard $(WASP_DIR)/lib/libwasp*$(lib_suffix))
endif
wasp_LIBS          := $(notdir $(wasp_LIBS))
wasp_LIBS          := $(patsubst %.$(lib_suffix),%,$(wasp_LIBS))
wasp_LIBS          := $(patsubst lib%,-l%,$(wasp_LIBS))
ifeq ($(wasp_LIBS),)
  $(error WASP does not seem to be available. Make sure to either run scripts/update_and_rebuild_wasp.sh in your MOOSE directory, or set WASP_DIR to a valid WASP install)
endif

wasp_CXXFLAGS      += -I$(WASP_DIR)/include
wasp_LDFLAGS       += -Wl,-rpath,$(WASP_DIR)/lib -L$(WASP_DIR)/lib $(wasp_LIBS)

# ---- GTEST files -------------------------------------------------------
DEPS_DIR           := .depend
BUILD_DIR          := build

GTEST_VERSION      := v1.17.0
GTEST_URL          := https://github.com/google/googletest/archive/refs/tags/$(GTEST_VERSION).tar.gz
GTEST_TAR          := $(DEPS_DIR)/gtest.tar.gz
GTEST_DIR          := $(DEPS_DIR)/googletest-$(GTEST_VERSION)
GTEST_SRC_DIR      := $(GTEST_DIR)/googletest

GTEST_INC_FLAGS    := -I$(GTEST_SRC_DIR)/include -I$(GTEST_SRC_DIR)
CPPFLAGS           += -isystem $(GTEST_SRC_DIR)/include

# ---- Targets -----------------------------------------------------------

.PHONY: all clean test

# Default is to just build executable
all: test_hit

# We have decided to get a fresh version of GTEST instead of using the contrib
# version in MOOSE
$(GTEST_DIR):
	@mkdir -p $(DEPS_DIR)
	@echo "Downloading gtest from ${GTEST_URL}..."
	curl -L "$(GTEST_URL)" -o $(GTEST_TAR)
	@echo "Extracting gtest archive from ${GTEST_TAR} to $(GTEST_DIR)..."
	@mkdir -p $@
	tar -xzf $(GTEST_TAR) -C $(GTEST_DIR) --strip-components=1

# Build GTEST API objects.
$(BUILD_DIR)/gtest-all.o: $(GTEST_DIR)
	@mkdir -p $(BUILD_DIR)
	$(hit_CXX) $(STD) $(CPPFLAGS) $(GTEST_INC_FLAGS) $(CXXFLAGS) \
		-c $(GTEST_SRC_DIR)/src/gtest-all.cc -o $@

# Build GTEST main function.
$(BUILD_DIR)/gtest_main.o: $(GTEST_DIR)
	@mkdir -p $(BUILD_DIR)
	$(hit_CXX) $(STD) $(CPPFLAGS) $(GTEST_INC_FLAGS) $(CXXFLAGS) \
		-c $(GTEST_SRC_DIR)/src/gtest_main.cc -o $@

# Building with gtest_main allows us to not have main function in our suite
$(BUILD_DIR)/gtest_main.a: $(BUILD_DIR)/gtest-all.o $(BUILD_DIR)/gtest_main.o
	@ar rcs $@ $^

# Build test object
$(BUILD_DIR)/test_hit.o: test_hit.cc $(GTEST_DIR)
	@mkdir -p $(BUILD_DIR)
	$(hit_CXX) $(STD) $(CPPFLAGS) $(GTEST_INC_FLAGS) $(wasp_CXXFLAGS) $(hit_includeflags) \
		$(CXXFLAGS) -c $< -o $@

# Build test executable
test_hit: $(BUILD_DIR)/test_hit.o $(BUILD_DIR)/gtest_main.a
	$(hit_CXX) $(STD) $(CPPFLAGS) $(GTEST_INC_FLAGS) $(wasp_CXXFLAGS) $(hit_includeflags) \
		$(CXXFLAGS) $(hit_srcfiles) -lpthread $^ $(wasp_LDFLAGS) -o $@

# Optional target to run tests once executable is built
test: test_hit
	./test_hit


clean:
	rm -rf $(BUILD_DIR) $(DEPS_DIR) test_hit
