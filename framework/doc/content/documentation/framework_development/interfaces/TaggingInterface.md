# TaggingInterface

TaggingInterface stores the basic information used for controlling how to contribute local element
residuals/Jacobians to multiple global vectors/matrices. Regular Kernels, Boundary conditions, Scalar Kernels, DG Kernels etc. should inherit from TaggingInterface.

## Vector tags

By default objects contribute to the `nontime` vector and the `system` matrix.  `TimeKernel` derived objects automatically contribute to the `time` vector.

This behavior can be modified by setting parameters in the object's blocks in the input file:

| Tag           | Description |
|-              |            -|
|vector_tags    | The tag for the vectors this object should fill |
|extra_vector_tags | Add more vectors to fill beyond what is in `vector_tags` - useful for adding without overriding the defaults|
|matrix_tags    | The tag for the matrices this object should fill |
|extra_matrix_tags | Add more matrices to fill beyond what is in `matrix_tags` - useful for adding without overriding the defaults |
