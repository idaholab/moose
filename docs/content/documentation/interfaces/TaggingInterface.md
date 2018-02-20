## TaggingInterface
TaggingInterface stores the basic information used for controlling how to contribute local element
residuals/Jacobians to multiple global vectors/matrices. Regular Kernels, Boundary conditions, Scalar kernels, DG Kernels etc. should inherit from TaggingInterface.

### Vector tags
The _nontime_ vector and matrix tags is assigned by default. This could be changed in _kernel_ block in a input file by setting the parameters

| Tag           | Description |
|-              |            -|
|vector_tags    | The tag for the vectors this Kernel should fill |
|matrix_tags    | The tag for the matrices this Kernel should fill |
