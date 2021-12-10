# Looking for the installation (maybe it is in the default location)
ifeq ($(TORCH_DIR),)
	TORCH_DIR ?= $(APPLICATION_DIR)/libtorch
endif

ifneq ($(TORCH_DIR),)
  ifneq ($(wildcard $(TORCH_DIR)/lib/libtorch.so),)

	  # Enabling parts that have pytorch dependencies
		libmesh_CXXFLAGS += -DPYTOCH_ENABLED

  	# Adding the include directories
		libmesh_CXXFLAGS += -I$(TORCH_DIR)/include/torch/csrc/api/include
		libmesh_CXXFLAGS += -I$(TORCH_DIR)/include

		# Dynamically linking with the available pytorch library
		libmesh_LDFLAGS += -Wl,-rpath,$(TORCH_DIR)/lib
		libmesh_LDFLAGS += -L$(TORCH_DIR)/lib -ltorch
  else
    $(info ERROR! Could not find any dynamic libraries of libtorch.)
    $(info Please check the following directory $(TORCH_DIR)/lib)
		$(info Build will proceed without the compilation of libtorch-related parts.)
	endif

else
	$(info ERROR! Could not find active libtorch installation.)
	$(info Try to run $(APPLICATION_DIR)/scripts/setup_pytorch.sh before calling make!)
	$(info Build will proceed without the compilation of libtorch-related parts.)
endif
