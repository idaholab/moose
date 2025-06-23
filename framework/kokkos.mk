# CUDA architecture strings
KOKKOS_CUDA_ARCH_30 := Kepler30
KOKKOS_CUDA_ARCH_32 := Kepler32
KOKKOS_CUDA_ARCH_35 := Kepler35
KOKKOS_CUDA_ARCH_37 := Kepler37
KOKKOS_CUDA_ARCH_50 := Maxwell50
KOKKOS_CUDA_ARCH_52 := Maxwell52
KOKKOS_CUDA_ARCH_53 := Maxwell53
KOKKOS_CUDA_ARCH_60 := Pascal60
KOKKOS_CUDA_ARCH_61 := Pascal61
KOKKOS_CUDA_ARCH_70 := Volta70
KOKKOS_CUDA_ARCH_72 := Volta72
KOKKOS_CUDA_ARCH_75 := Turing75
KOKKOS_CUDA_ARCH_80 := Ampere80
KOKKOS_CUDA_ARCH_86 := Ampere86
KOKKOS_CUDA_ARCH_89 := Ada89
KOKKOS_CUDA_ARCH_90 := Hopper90

CUDA_COMPILER ?= nvcc
HIP_COMPILER ?= hipcc
SYCL_COMPILER ?= dpcpp

CUDA_EXISTS := $(shell which ${CUDA_COMPILER} 2>/dev/null)
HIP_EXISTS := $(shell which ${HIP_COMPILER} 2>/dev/null)
SYCL_EXISTS := $(shell which ${SYCL_COMPILER} 2>/dev/null)

PETSC_CONF := $(shell \
  for path in $(patsubst -I%,%,$(libmesh_INCLUDE)); do \
    if test -f "$$path/petscconf.h"; then \
      echo $$path/petscconf.h; \
      break; \
    fi; \
  done; \
)

PETSC_HAVE_KOKKOS := $(shell sed -n 's/\#define PETSC_HAVE_KOKKOS //p' $(PETSC_CONF))
PETSC_HAVE_CUDA := $(shell sed -n 's/\#define PETSC_HAVE_CUDA //p' $(PETSC_CONF))

ifeq ($(PETSC_HAVE_KOKKOS),1)
  KOKKOS_PATH :=
  ifeq ($(PETSC_HAVE_CUDA),1)
    PETSC_HAVE_CUDA_MIN_ARCH := $(shell sed -n 's/\#define PETSC_HAVE_CUDA_MIN_ARCH //p' $(PETSC_CONF))
    PETSC_PKG_CUDA_MIN_ARCH := $(shell sed -n 's/\#define PETSC_PKG_CUDA_MIN_ARCH //p' $(PETSC_CONF))
    ifneq ($(PETSC_HAVE_CUDA_MIN_ARCH),)
      ifneq ($(CUDA_ARCH),)
        $(warning Provided CUDA_ARCH ($(CUDA_ARCH)) will be ignored and PETSC_HAVE_CUDA_MIN_ARCH ($(PETSC_HAVE_CUDA_MIN_ARCH)) will be used)
      endif
      CUDA_ARCH := $(PETSC_HAVE_CUDA_MIN_ARCH)
    endif
    ifneq ($(PETSC_PKG_CUDA_MIN_ARCH),)
      ifneq ($(CUDA_ARCH),)
        $(warning Provided CUDA_ARCH ($(CUDA_ARCH)) will be ignored and PETSC_PKG_CUDA_MIN_ARCH ($(PETSC_PKG_CUDA_MIN_ARCH)) will be used)
      endif
      CUDA_ARCH := $(PETSC_PKG_CUDA_MIN_ARCH)
    endif
  endif
else
  # We support using Kokkos through submodule as a fallback, but we will error until we can test it
  $(error PETSc was not configured with Kokkos. Recompile PETSc with Kokkos support)
  KOKKOS_PATH := $(FRAMEWORK_DIR)/contrib/kokkos
endif

ifneq ($(KOKKOS_PATH),)
  ifeq ($(wildcard $(KOKKOS_PATH)/Makefile.kokkos),)
    $(error Kokkos submodule was not initialized)
  endif
endif

CXXFLAGS += -DMOOSE_HAVE_GPU

KOKKOS_CPPFLAGS = $(libmesh_CPPFLAGS) $(ADDITIONAL_CPPFLAGS)
KOKKOS_LIBS = $(libmesh_LIBS)

ifneq ($(CUDA_EXISTS),)
  ifeq ($(CUDA_ARCH),)
    CUDA_ARCH := $(shell nvidia-smi -i 0 --query-gpu=compute_cap --format=csv | tr -cd '0-9')
    ifeq ($(KOKKOS_CUDA_ARCH_$(CUDA_ARCH)),)
      $(error CUDA_ARCH cannot be determined automatically and should be set manually)
    endif
  endif
  KOKKOS_CXX := nvcc
  KOKKOS_DEVICES := Cuda
  KOKKOS_CXXFLAGS = --forward-unknown-to-host-compiler --disable-warnings -x cu -ccbin $(word 1, $(libmesh_CXX))
  KOKKOS_CXXFLAGS += $(filter-out -Werror=return-type -Werror=reorder,$(libmesh_CXXFLAGS)) # Incompatible with NVCC
  KOKKOS_LDFLAGS = --forward-unknown-to-host-compiler
  KOKKOS_ARCH_STRING := $(KOKKOS_CUDA_ARCH_$(CUDA_ARCH))
  ifeq ($(KOKKOS_ARCH_STRING),)
    $(error Unsupported CUDA_ARCH ($(CUDA_ARCH)) for Kokkos)
  endif
  ifeq ($(KOKKOS_PATH),)
    KOKKOS_CXXFLAGS += -arch=sm_$(CUDA_ARCH)
  else
    KOKKOS_ARCH := $(KOKKOS_ARCH_STRING)
  endif
else ifneq ($(HIP_EXISTS),) # To be determined for HIP
  ifeq ($(shell hipconfig --platform), nvidia)
    $(error For NVIDIA GPUs, use CUDA instead of HIP for Kokkos)
  endif
  KOKKOS_CXX := hipcc
  KOKKOS_DEVICES := HIP
else ifneq ($(SYCL_EXISTS),) # To be determined for SYCL
  KOKKOS_CXX := dpcpp
  KOKKOS_DEVICES := SYCL
else
  $(error No suitable GPU SDK was found for Kokkos)
endif

ifneq ($(KOKKOS_PATH),)
  CXX := $(KOKKOS_CXX)
endif

ifeq ($(METHOD),dbg)
  KOKKOS_DEBUG := yes
else ifeq ($(METHOD),devel)
  KOKKOS_DEBUG := yes
else ifeq ($(METHOD),opt)
  KOKKOS_DEBUG := no
  KOKKOS_CXXFLAGS += -DNDEBUG
endif

KOKKOS_CXXFLAGS += $(CXXFLAGS) -fPIC -DMOOSE_GPU_SCOPE
KOKKOS_LDFLAGS += $(libmesh_LDFLAGS)

ifneq ($(KOKKOS_PATH),)
  include $(KOKKOS_PATH)/Makefile.kokkos
endif

KOKKOS_OBJ_SUFFIX := $(libmesh_HOST).$(METHOD).o

%.$(KOKKOS_OBJ_SUFFIX) : %.K $(KOKKOS_CPP_DEPENDS)
	@echo "Compiling Kokkos C++ (in "$(METHOD)" mode, $(KOKKOS_DEVICES), $(KOKKOS_ARCH_STRING)) "$<"..."
	@$(KOKKOS_CXX) $(KOKKOS_CPPFLAGS) $(KOKKOS_CXXFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) -MMD -MP -MF $@.d -MT $@ -c $< -o $@

ifneq ($(KOKKOS_PATH),)
  KOKKOS_CLEAN := kokkos-clean
endif
