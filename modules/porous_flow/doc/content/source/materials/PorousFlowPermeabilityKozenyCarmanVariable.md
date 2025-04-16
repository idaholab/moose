# PorousFlowPermeabilityKozenyCarmanVariable

!syntax description /Materials/PorousFlowPermeabilityKozenyCarmanVariable

Permeability is calculated from porosity using
\begin{equation}
k_{ij} = A k_{ij}^{0} \frac{\phi^{n}}{(1 - \phi)^{m}},
\end{equation}
where $n$ and $m$ are user-defined constants and $A$ is a given by a variable.  See [PorousFlowPermeabilityKozenyCarman.md] for functional forms commonly used for $A$.  Spatially varying `A` allows us to capture spatially varying changes in aperature according to the permeability and porosity equations given in the Materials properties section of [multiapp_fracture_flow_PorousFlow_3D.md].

## Example Input File Syntax

In the following example, `A_var` is a spatially varying `AuxVariable` that produces the same value for $A$ as the `permeability_0` and `permeability_1` materials.  The value for `A_var` is computed from $A = k_0 (1 - \phi_0)^m / \phi_0^n$ which is equivalent to a [PorousFlowPermeabilityKozenyCarman.md] material with `poroperm_function=kozeny_carman_phi0`.

!listing modules/porous_flow/test/tests/poroperm/PermFromPoro02.i block=Materials/permeability_all

!syntax parameters /Materials/PorousFlowPermeabilityKozenyCarmanVariable

!syntax inputs /Materials/PorousFlowPermeabilityKozenyCarmanVariable

!syntax children /Materials/PorousFlowPermeabilityKozenyCarmanVariable
