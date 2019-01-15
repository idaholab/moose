# Compute Interface Stress KKS

!syntax description /Materials/ComputeInterfaceStressKKS

## Description

This material generates a surface stress tensor in the interface between two phases
in the Kim-Kim-Suzuki (KKS) phase-field model ([cite:kim_phase-field_1999]).
The surface stress consists of two orthogonal principal components lying in plane
of the interface. The approach is based on the formulation described in [cite:Levitas2011].

The surface stress tensor $\mathbf{\sigma}_{st}$ is given by
\begin{equation}
\mathbf{\sigma}_{st} = \left[W g(\eta) + \frac \kappa 2 |\nabla \eta|^2 \right]
\bf{I} - \kappa \nabla \eta \otimes \nabla \eta
\end{equation}
where $W$ is the free energy barrier between phases, $g(\eta)$ is the double-well
function, $\eta$ is the order parameter, $\kappa$ is the gradient energy
coefficient, $\bf{I}$ is the identity tensor, and $\otimes$ denotes the dyadic product.

## Example Input File Syntax

!listing modules/combined/test/tests/interface_stress_KKS/interface_stress_KKS.i block=Materials/interface_stress

!syntax parameters /Materials/ComputeInterfaceStressKKS

!syntax inputs /Materials/ComputeInterfaceStressKKS

!syntax children /Materials/ComputeInterfaceStressKKS

!bibtex bibliography
