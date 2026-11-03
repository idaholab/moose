# Depth of subfolders used to create unity groups
app_unity_depth = 2

# The Torch FEM kernels under src/libtorch pull in libtorch headers, which must not be unity-built.
ifeq ($(ENABLE_LIBTORCH),true)
  libtorch_dirs := $(shell find $(APPLICATION_DIR)/src/libtorch -type d -not -path '*/.libs*' 2> /dev/null)
  app_non_unity_dirs += $(libtorch_dirs)
endif
