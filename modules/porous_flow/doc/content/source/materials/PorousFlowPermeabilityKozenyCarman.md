# PorousFlowPermeabilityKozenyCarman

!syntax description /Materials/PorousFlowPermeabilityKozenyCarman

Permeability is calculated from porosity using
\begin{equation}
k_{ij} = A k_{ij}^{0} \frac{\phi^{n}}{(1 - \phi)^{m}},
\end{equation}
where $n$ and $m$ are user-defined constants.

Input can be entered in one of two forms depending on the value of `poroperm_function`

!table caption=Input format
| `poroperm_function` | input format |
| --- | --- |
| `kozeny_carman_fd2` | $A = f d^2$ |
| `kozeny_carman_fd2` | $A = k_0 (1 - \phi)^m / \phi^n$  |

The parameters $A$ and $B$ are then converted to the correct form internally.

!syntax parameters /Materials/PorousFlowPermeabilityKozenyCarman

!syntax inputs /Materials/PorousFlowPermeabilityKozenyCarman

!syntax children /Materials/PorousFlowPermeabilityKozenyCarman
