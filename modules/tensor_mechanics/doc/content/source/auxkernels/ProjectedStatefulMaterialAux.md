# ProjectedStatefulMaterialAux

This AuxKernel simply returns the value of a given material property at a quadrature point with the purpose of projecting the property onto an elemental basis function (e.g. first order monomial).

At step zero this object will compute the material QP values by explicitly calling `initStatefulProperties` in order to project the state that will correspond to the *old state* in the first timestep.

This object is set up by the [ProjectedStatefulMaterialStorageAction](ProjectedStatefulMaterialStorageAction.md).

!syntax description /AuxKernels/ProjectedStatefulMaterialAux

!syntax parameters /AuxKernels/ProjectedStatefulMaterialAux

!syntax inputs /AuxKernels/ProjectedStatefulMaterialAux

!syntax children /AuxKernels/ProjectedStatefulMaterialAux
