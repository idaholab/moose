# PorousFlowPermeabilityKozenyCarman

!syntax description /Materials/PorousFlowPermeabilityKozenyCarman

Permeability is calculated from porosity using
\begin{equation}
k_{ij} = A k_{ij}^{0} \frac{\phi^{n}}{(1 - \phi)^{m}},
\end{equation}
where $n$ and $m$ are user-defined constants.

Input can be entered in one of two forms depending on the value of [!param](/Materials/PorousFlowPermeabilityKozenyCarman/poroperm_function)

!table caption=Input format
| `poroperm_function` | input format |
| --- | --- |
| `kozeny_carman_fd2` | $A = f d^2$ |
| `kozeny_carman_phi0` | $A = k_0 (1 - \phi_0)^m / \phi_0^n$  |

In `kozeny_carman_fd2`, $f=$[!param](/Materials/PorousFlowPermeabilityKozenyCarman/f) is a scalar constant and $d=$[!param](/Materials/PorousFlowPermeabilityKozenyCarman/d) is the grain diameter.  In `kozeny_carman_phi0`, $k_0=$[!param](/Materials/PorousFlowPermeabilityKozenyCarman/k0) is the reference permeability and $\phi_0=$[!param](/Materials/PorousFlowPermeabilityKozenyCarman/phi0) is a reference porosity.  These input parameters are converted to $A$ internally.  For problems where $A$ is described by a spatially varying variable, use `[PorousFlowPermeabilityKozenyCarmanVariable.md] .


!syntax parameters /Materials/PorousFlowPermeabilityKozenyCarman

!syntax inputs /Materials/PorousFlowPermeabilityKozenyCarman

!syntax children /Materials/PorousFlowPermeabilityKozenyCarman
