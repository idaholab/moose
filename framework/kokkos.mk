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
HIP_COMPILER  ?= hipcc
SYCL_COMPILER ?= dpcpp

CUDA_EXISTS := $(shell which ${CUDA_COMPILER} 2>/dev/null)
HIP_EXISTS  := $(shell which ${HIP_COMPILER} 2>/dev/null)
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
PETSC_HAVE_CUDA   := $(shell sed -n 's/\#define PETSC_HAVE_CUDA //p' $(PETSC_CONF))

ifeq ($(PETSC_HAVE_KOKKOS),1)
  ifeq ($(PETSC_HAVE_CUDA),1)
    PETSC_HAVE_CUDA_MIN_ARCH := $(shell sed -n 's/\#define PETSC_HAVE_CUDA_MIN_ARCH //p' $(PETSC_CONF))
    PETSC_PKG_CUDA_MIN_ARCH := $(shell sed -n 's/\#define PETSC_PKG_CUDA_MIN_ARCH //p' $(PETSC_CONF))
    ifneq ($(PETSC_HAVE_CUDA_MIN_ARCH),)
      CUDA_ARCH := $(PETSC_HAVE_CUDA_MIN_ARCH)
    endif
    ifneq ($(PETSC_PKG_CUDA_MIN_ARCH),)
      CUDA_ARCH := $(PETSC_PKG_CUDA_MIN_ARCH)
    endif
  endif
else
  $(error PETSc was not configured with Kokkos support)
endif

CXXFLAGS += -DMOOSE_HAVE_KOKKOS

ifneq ($(CUDA_EXISTS),)
  KOKKOS_DEVICE   := CUDA
  KOKKOS_CXX      := nvcc
  KOKKOS_ARCH     := $(KOKKOS_CUDA_ARCH_$(CUDA_ARCH))
  KOKKOS_CXXFLAGS  = --forward-unknown-to-host-compiler --disable-warnings -x cu -ccbin $(word 1, $(libmesh_CXX))
  KOKKOS_CXXFLAGS += $(filter-out -Werror=return-type -Werror=reorder,$(libmesh_CXXFLAGS)) # Incompatible with NVCC
  KOKKOS_LDFLAGS   = --forward-unknown-to-host-compiler
  KOKKOS_CXXFLAGS += -arch=sm_$(CUDA_ARCH)
else ifneq ($(HIP_EXISTS),) # To be determined for HIP
  ifeq ($(shell hipconfig --platform), nvidia)
    $(error For NVIDIA GPUs, use CUDA instead of HIP for Kokkos)
  endif
  KOKKOS_DEVICE   := HIP
  KOKKOS_CXX      := hipcc
else ifneq ($(SYCL_EXISTS),) # To be determined for SYCL
  KOKKOS_DEVICE   := SYCL
  KOKKOS_CXX      := dpcpp
else
  $(error No suitable GPU SDK was found for Kokkos)
endif

ifeq ($(METHOD),opt)
  KOKKOS_CXXFLAGS += -DNDEBUG
endif

KOKKOS_CXXFLAGS += $(CXXFLAGS) -fPIC -DMOOSE_KOKKOS_SCOPE
KOKKOS_LDFLAGS  += $(libmesh_LDFLAGS)
KOKKOS_CPPFLAGS  = $(libmesh_CPPFLAGS) $(ADDITIONAL_CPPFLAGS)
KOKKOS_INCLUDE   = $(libmesh_INCLUDE)
KOKKOS_LIBS      = $(libmesh_LIBS)

KOKKOS_OBJ_SUFFIX := $(libmesh_HOST).$(METHOD).o

%.$(KOKKOS_OBJ_SUFFIX) : %.K $(KOKKOS_CPP_DEPENDS)
	@echo "Compiling Kokkos C++ (in "$(METHOD)" mode, $(KOKKOS_DEVICE), $(KOKKOS_ARCH)) "$<"..."
	@$(KOKKOS_CXX) $(KOKKOS_CPPFLAGS) $(KOKKOS_CXXFLAGS) $(KOKKOS_INCLUDE) $(app_INCLUDES) -MMD -MP -MF $@.d -MT $@ -c $< -o $@
