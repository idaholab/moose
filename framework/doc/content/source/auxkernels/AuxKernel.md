# AuxKernel

An `AuxKernel` computes values that are stored in "Auxiliary Variables".  There is an infinite number of ways to generate these values but some common ones are: based on other variables/data in the simulation (i.e. coupled data), read from an external file and interpolated from a Function.  The Auxiliary system is _extremely_ flexible on purpose.  The data values computed by `AuxKernel`s and stored in Auxiliary Variables (AuxVariables) are often used for visualization (i.e. written to output files) but can also be coupled back into other calculations (including in `Kernels`) or provided as the input to other systems such as `Postprocessors`.

For more information: see the [documentation for the Auxiliary System](AuxKernels/index.md)
