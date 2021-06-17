# FVOneVarDiffusionInterface

!syntax description /FVInterfaceKernels/FVOneVarDiffusionInterface

This class is only meant to be used with a single variable, e.g. the `variable1`
parameter should be provided but not `variable2`. `coeff1` should correspond to
the diffusion or conductivity coefficient on the `subdomain1` side of the
`boundary` while `coeff2` corresponds to `subdomain2`. The gradient of
`variable1` is based on Green-Gauss computations of the gradient at the
adjoining element centroids followed by linear interpolation to the face plus a
non-orthogonal correction. The diffusion/conductivity coefficient at the
interface is computed using linear interpolation of the `coeff1` and `coeff2`
values. This discretization scheme is O(h) accurate.

## Example input file syntax

!listing test/tests/fviks/one-var-diffusion/test.i block=FVInterfaceKernels/interface

!syntax parameters /FVInterfaceKernels/FVOneVarDiffusionInterface

!syntax inputs /FVInterfaceKernels/FVOneVarDiffusionInterface

!syntax children /FVInterfaceKernels/FVOneVarDiffusionInterface
