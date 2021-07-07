# PorousFlowElementNormal

This `AuxKernel` calculates the element normal, and returns the *x*, *y*, or *z* component.  This is mostly designed for 2D elements living in 3D space, however, the 1D and 3D cases are handled as special cases using the [!param](/AuxKernels/PorousFlowElementNormal/1D_perp) and [!param](/AuxKernels/PorousFlowElementNormal/3D_default) inputs.

For 2D elements, the normal is calculated by:

1. Construct the vector $(n_{2} - n_{1})\times (n_{0} - n_{1})$, where $\times$ is the cross-product, and $n_{i}$ is the position of the $i^{\mathrm{th}}$ node.
2. Construct the vector $(n_{3} - n_{2})\times (n_{1} - n_{2})$
3. $\ldots$
3. Construct the vector $(n_{N-1} - n_{N-2})\times (n_{N-3} - n_{N-2})$, where $N$ is the number of nodes in the element.  Hence, for triangular linear Lagrange elements ($N=3$) only 1 vector is constructed, for quadrilateral linear Lagrange elements ($N=4$) 2 vectors are constructed
4. Sum these vectors, normalise, and return the result

For 1D elements, the normal is calculated by:

1. Construct the vector $v\times (n_{1} - n_{0})$, where $v$ is the [!param](/AuxKernels/PorousFlowElementNormal/1D_perp) vector supplied by the user
2. Construct the vector $v\times (n_{2} - n_{1})$
3. $\ldots$
3. Construct the vector $v\times (n_{N-1} - n_{N-2})$, where $N$ is the number of nodes in the element.  Hence, for bar linear Lagrange elements ($N=2$) only 1 vector is constructed
4. Sum these vectors, normalise, and return the result

For 3D elements, the [!param](/AuxKernels/PorousFlowElementNormal/3D_default) is returned.


!alert note
Only elemental (`Monomial`) `AuxVariables` can be used with this `AuxKernel`

!syntax parameters /AuxKernels/PorousFlowElementNormal

!syntax inputs /AuxKernels/PorousFlowElementNormal

!syntax children /AuxKernels/PorousFlowElementNormal
