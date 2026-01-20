# CUDA architecture strings
KOKKOS_CUDA_ARCH_30  := Kepler CC 3.0
KOKKOS_CUDA_ARCH_32  := Kepler CC 3.2
KOKKOS_CUDA_ARCH_35  := Kepler CC 3.5
KOKKOS_CUDA_ARCH_37  := Kepler CC 3.7
KOKKOS_CUDA_ARCH_50  := Maxwell CC 5.0
KOKKOS_CUDA_ARCH_52  := Maxwell CC 5.2
KOKKOS_CUDA_ARCH_53  := Maxwell CC 5.3
KOKKOS_CUDA_ARCH_60  := Pascal CC 6.0
KOKKOS_CUDA_ARCH_61  := Pascal CC 6.1
KOKKOS_CUDA_ARCH_70  := Volta CC 7.0
KOKKOS_CUDA_ARCH_72  := Volta CC 7.2
KOKKOS_CUDA_ARCH_75  := Turing CC 7.5
KOKKOS_CUDA_ARCH_80  := Ampere CC 8.0
KOKKOS_CUDA_ARCH_86  := Ampere CC 8.6
KOKKOS_CUDA_ARCH_89  := Ada CC 8.9
KOKKOS_CUDA_ARCH_90  := Hopper CC 9.0
KOKKOS_CUDA_ARCH_100 := Blackwell CC 10.0
KOKKOS_CUDA_ARCH_120 := Blackwell CC 12.0

CUDA_COMPILER ?= nvcc
HIP_COMPILER  ?= hipcc
SYCL_COMPILER ?= icpx

PETSC_CONF := $(shell \
  for path in $(patsubst -I%,%,$(libmesh_INCLUDE)); do \
    if test -f "$$path/petscconf.h"; then \
      echo $$path/petscconf.h; \
      break; \
    fi; \
  done; \
)

PETSC_HAVE_KOKKOS  := $(shell sed -n 's/\#define PETSC_HAVE_KOKKOS //p' $(PETSC_CONF))
PETSC_HAVE_CUDA    := $(shell sed -n 's/\#define PETSC_HAVE_CUDA //p' $(PETSC_CONF))
PETSC_HAVE_HIP     := $(shell sed -n 's/\#define PETSC_HAVE_HIP //p' $(PETSC_CONF))
PETSC_HAVE_HIPCUDA := $(shell sed -n 's/\#define PETSC_HAVE_HIPCUDA //p' $(PETSC_CONF))
PETSC_HAVE_SYCL    := $(shell sed -n 's/\#define PETSC_HAVE_SYCL //p' $(PETSC_CONF))

ifeq ($(PETSC_HAVE_KOKKOS),1)
  ifeq ($(PETSC_HAVE_CUDA),1)
    CUDA_ARCH := $(shell sed -n 's/\#define PETSC_PKG_CUDA_MIN_ARCH //p' $(PETSC_CONF))
  endif
  ifeq ($(PETSC_HAVE_HIP),1)
    ifeq ($(PETSC_HAVE_HIPCUDA),1)
      $(error For NVIDIA GPUs, use CUDA instead of HIP)
    endif
  endif
  ifeq ($(PETSC_HAVE_SYCL),1)
  endif
else
  $(error PETSc was not configured with Kokkos support)
endif

ifeq ($(PETSC_HAVE_CUDA),1)
  ifeq ($(shell which ${CUDA_COMPILER} 2>/dev/null),)
    $(error CUDA compiler cannot be located. Set CUDA_COMPILER correctly (current value: $(CUDA_COMPILER)))
  endif
endif
ifeq ($(PETSC_HAVE_HIP),1)
  ifeq ($(shell which ${HIP_COMPILER} 2>/dev/null),)
    $(error HIP compiler cannot be located. Set HIP_COMPILER correctly (current value: $(HIP_COMPILER)))
  endif
endif
ifeq ($(PETSC_HAVE_SYCL),1)
  ifeq ($(shell which ${SYCL_COMPILER} 2>/dev/null),)
    $(error SYCL compiler cannot be located. Set SYCL_COMPILER correctly (current value: $(SYCL_COMPILER)))
  endif
endif

ifneq ($(PETSC_HAVE_CUDA),)
  KOKKOS_DEVICE     := CUDA
  KOKKOS_ARCH       := $(KOKKOS_CUDA_ARCH_$(CUDA_ARCH))
  KOKKOS_COMPILER   := GPU
  KOKKOS_CXX         = $(CUDA_COMPILER)
  KOKKOS_CXXFLAGS    = -arch=sm_$(CUDA_ARCH) --extended-lambda
  KOKKOS_CXXFLAGS   += --forward-unknown-to-host-compiler --disable-warnings -x cu -ccbin $(word 1, $(libmesh_CXX))
  KOKKOS_CXXFLAGS   += $(filter-out -Werror=return-type,$(CXXFLAGS) $(libmesh_CXXFLAGS)) # Incompatible with NVCC
  KOKKOS_CPPFLAGS    = $(subst -Werror,-Werror=all-warnings,$(libmesh_CPPFLAGS) $(ADDITIONAL_CPPFLAGS) ${ADDITIONAL_KOKKOS_CPPFLAGS})
  KOKKOS_LDFLAGS     = --forward-unknown-to-host-compiler -arch=sm_$(CUDA_ARCH)
else ifneq ($(PETSC_HAVE_HIP),) # To be determined for HIP
  KOKKOS_DEVICE     := HIP
  KOKKOS_ARCH       :=
  KOKKOS_COMPILER   := GPU
  KOKKOS_CXX         = $(HIP_COMPILER)
  KOKKOS_CXXFLAGS    =
  KOKKOS_CPPFLAGS    =
  KOKKOS_LDFLAGS     =
else ifneq ($(PETSC_HAVE_SYCL),) # To be determined for SYCL
  KOKKOS_DEVICE     := SYCL
  KOKKOS_ARCH       :=
  KOKKOS_COMPILER   := GPU
  KOKKOS_CXX         = $(SYCL_COMPILER)
  KOKKOS_CXXFLAGS    = -fsycl
  KOKKOS_CPPFLAGS    =
  KOKKOS_LDFLAGS     =
else
  KOKKOS_COMPILER   := CPU
  KOKKOS_CXX         = $(libmesh_CXX)
  KOKKOS_CXXFLAGS    = $(CXXFLAGS) $(libmesh_CXXFLAGS) -x c++
  KOKKOS_CPPFLAGS    = $(libmesh_CPPFLAGS) $(ADDITIONAL_CPPFLAGS) ${ADDITIONAL_KOKKOS_CPPFLAGS}
  KOKKOS_LDFLAGS     =
  ENABLE_KOKKOS_GPU := false
endif

ifeq ($(ENABLE_KOKKOS_GPU),false)
  KOKKOS_DEVICE := CPU
  KOKKOS_ARCH   := OpenMP
else
  CXXFLAGS      += -DMOOSE_ENABLE_KOKKOS_GPU=1
endif

KOKKOS_CXXFLAGS += -DMOOSE_KOKKOS_SCOPE=1 -fPIC
KOKKOS_LDFLAGS  += $(libmesh_LDFLAGS)
KOKKOS_INCLUDE   = $(libmesh_INCLUDE)
KOKKOS_LIBS      = $(libmesh_LIBS)

ifeq ($(METHOD),opt)
  KOKKOS_CXXFLAGS += -DNDEBUG
endif

ifeq ($(KOKKOS_COMPILER),CPU)

KOKKOS_LIB_SUFFIX := _kokkos-$(METHOD).la
KOKKOS_OBJ_SUFFIX := $(libmesh_HOST).$(METHOD).lo

%.$(KOKKOS_OBJ_SUFFIX) : %.K
	@echo "Compiling Kokkos C++ (in "$(METHOD)" mode, $(KOKKOS_DEVICE), $(KOKKOS_ARCH)) "$<"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile --quiet \
	  $(KOKKOS_CXX) $(KOKKOS_CXXFLAGS) $(KOKKOS_CPPFLAGS) $(KOKKOS_INCLUDE) $(app_INCLUDES) -MMD -MP -MF $@.d -MT $@ -c $< -o $@

else

KOKKOS_LIB_SUFFIX := _kokkos-$(METHOD).so
KOKKOS_OBJ_SUFFIX := $(libmesh_HOST).$(METHOD).o

%.$(KOKKOS_OBJ_SUFFIX) : %.K
	@echo "Compiling Kokkos C++ (in "$(METHOD)" mode, $(KOKKOS_DEVICE), $(KOKKOS_ARCH)) "$<"..."
	@$(KOKKOS_CXX) $(KOKKOS_CXXFLAGS) $(KOKKOS_CPPFLAGS) $(KOKKOS_INCLUDE) $(app_INCLUDES) -MMD -MP -MF $@.d -MT $@ -c $< -o $@

endif
