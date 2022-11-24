ifeq ($(ENABLE_LIBTORCH),true)
  libtorch_dirs := $(shell find src/libtorch -type d -not -path '*/.libs*' 2> /dev/null)
  converted_dirs := $(foreach i, $(libtorch_dirs), %$(i))
  app_non_unity_dirs = $(converted_dirs)
endif
