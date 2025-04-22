# PorousFlowPermeabilityKozenyCarman

!syntax description /Materials/PorousFlowPermeabilityKozenyCarman

Permeability is calculated from porosity using
\begin{equation}
k_{ij} = A k_{ij}^{0} \frac{\phi^{n}}{(1 - \phi)^{m}},
\end{equation}
where $n$ and $m$ are user-defined constants.

Input for computing $A$ can be entered in one of three forms depending on the value of [!param](/Materials/PorousFlowPermeabilityKozenyCarman/poroperm_function)

!table caption=Input format
| `poroperm_function` | input format |
| --- | --- |
| `kozeny_carman_fd2` | $A = f d^2$ |
| `kozeny_carman_phi0` | $A = k_0 (1 - \phi_0)^m / \phi_0^n$  |
| `kozeny_carman_A` | $A = A$  |

For all methods, parameters must be chosen to make $A>0$.  In `kozeny_carman_fd2`, $f=$[!param](/Materials/PorousFlowPermeabilityKozenyCarman/f) is a scalar constant and $d=$[!param](/Materials/PorousFlowPermeabilityKozenyCarman/d) is the grain diameter.  In `kozeny_carman_phi0`, $k_0=$[!param](/Materials/PorousFlowPermeabilityKozenyCarman/k0) is the reference permeability and $\phi_0=$[!param](/Materials/PorousFlowPermeabilityKozenyCarman/phi0) is a reference porosity.  These input parameters are converted to $A$ internally.  In `kozeny_carman_A`, $A=$[!param](/Materials/PorousFlowPermeabilityKozenyCarman/A) is used to provide the permeability multilpying factor directly which is useful for 2D fracture flow described [here](multiapp_fracture_flow_PorousFlow_3D.md) where $A$ is a constant given by $A=r/12$ with $r$ being the fracture roughness. For problems where $A$ is described by a spatially varying variable, use `[PorousFlowPermeabilityKozenyCarmanFromVar.md].

!syntax parameters /Materials/PorousFlowPermeabilityKozenyCarman

!syntax inputs /Materials/PorousFlowPermeabilityKozenyCarman

!syntax children /Materials/PorousFlowPermeabilityKozenyCarman
