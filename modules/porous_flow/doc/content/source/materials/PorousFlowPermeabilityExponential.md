# PorousFlowPermeabilityExponential

!syntax description /Materials/PorousFlowPermeabilityExponential

A simple porosity-permeability model where
\begin{equation*}
k_{ij} = B k_{ij}^{0} e^{A \phi},
\end{equation*}
where $\phi$ is the porosity, and $A$ and $B$ are user-defined constant.

Input can be entered in any of three forms depending on the value of `poroperm_function`

!table caption=Input format
| `poroperm_function` | input format |
| --- | --- |
| `log_k` | $\log k = A \phi + B$ |
| `\ln k` | $\ln k = A \phi + B$  |
| `k`     | $k = B \exp(A \phi)$  |

The parameters $A$ and $B$ are then converted to the correct form internally.

!syntax parameters /Materials/PorousFlowPermeabilityExponential

!syntax inputs /Materials/PorousFlowPermeabilityExponential

!syntax children /Materials/PorousFlowPermeabilityExponential
