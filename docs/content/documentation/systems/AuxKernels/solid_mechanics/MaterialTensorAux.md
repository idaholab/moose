# MaterialTensorAux
!syntax description /AuxKernels/MaterialTensorAux

## Description
The `MaterialTensorAux` AuxKernel is used to output quantities related to second-order
tensors used as material properties.  Stress and strain are common examples of these
tensors.  The AuxKernel allows output of specific tensor entries or quantities computed
from the entire tensor.  Typically, the AuxVariable computed by `MaterialTensorAux` will
be an element-level, constant variable. By default, the computed value will be the volume-
averaged quantity over the element.  If the parameter qp_select is set to the value of an
integration point number (0, 1, ..., n), the computed valued will be the value at that
integration point.

!syntax parameters /AuxKernels/MaterialTensorAux

!syntax inputs /AuxKernels/MaterialTensorAux

!syntax children /AuxKernels/MaterialTensorAux
