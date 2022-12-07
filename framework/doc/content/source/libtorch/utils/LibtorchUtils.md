# LibtorchUtils Namespace

MOOSE includes a number of C++ utility classes and functions that may be useful for
Manipulating Libtorch tensors and interfacing them with MOOSE-based objects.

## Standard Vector To Tensor and Tensor to Standard Vector conversion

The utility classes streamline the two-way conversion between standard vectors
and Libtorch tensors.

!listing framework/include/libtorch/utils/LibtorchUtils.h 
 start=/**
 end=tensorToVector
 include-end=true
