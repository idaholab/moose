# DSGShellNodalResultants

!syntax description /UserObjects/DSGShellNodalResultants

## Description

This user object recovers the stress resultants of the DSG shell elements at the nodes. Each element evaluates its generalized strains at its own nodes from its own interpolation, the strain-displacement matrix of [ComputeDSGShellStrain](/ComputeDSGShellStrain.md) at the parent coordinates of the node, and [ComputeDSGShellResultants](/ComputeDSGShellResultants.md) turns them into resultants. This object averages, node by node, the values of the elements that share the node. The average is a plain one over the elements, the usual nodal averaging of finite element post-processors, and it is the same on any number of processors.

The strain material must be asked for the nodal strains with `compute_nodal_strains = true`; the user object reports an error otherwise. On a mesh with blocks of other elements, restrict the user object and the auxiliary kernel to the shell blocks with `block`, since the other elements carry no resultants. In parallel each processor averages its own elements and sends the sums of the nodes it does not own to their owner, so only the nodes on processor boundaries travel; the two-processor specification of the battery reproduces the one-processor values. The averaged values are read by [DSGShellNodalResultantsAux](/DSGShellNodalResultantsAux.md), one resultant per nodal auxiliary variable.

The recovery works on any mesh of the family, including shells in three-dimensional space, because it uses the element's own frame and interpolation and no fit over the neighbouring elements. On the linear members the curvatures are constant along each element, so the nodal value at a node shared by two elements is the mean of their values, exact for a linearly varying moment; on the quadratic members the moment within the element is linear and the nodal values are exact at every node, the supported edges included.

## Example Input File syntax

!listing modules/solid_mechanics/test/tests/shell_dsg/static/nodal_resultants.i block=Materials/strain UserObjects AuxKernels/moment_22_nodal

!syntax parameters /UserObjects/DSGShellNodalResultants

!syntax inputs /UserObjects/DSGShellNodalResultants

!syntax children /UserObjects/DSGShellNodalResultants
