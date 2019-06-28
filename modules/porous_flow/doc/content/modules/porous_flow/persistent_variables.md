# Primary variables

In a miscible multiphase simulation, where phases can appear/disappear depending
on the thermodynamical conditions, the appropriate primary nonlinear variables can
depend on the phase state of the system. For example, consider a two-phase, two-component
model. If both fluid phases are present, a suitable set of primary variables are
pressure of one phase, saturation of one phase, and temperature. If one phase disappears
(for instance, due to dissolution of the gas fluid component into the liquid fluid phase),
then phase saturation is no longer present in the governing equations, and is therefore
not an appropriate primary variable. In this case, the mass fraction of a fluid component
in the remaining phase is a suitable choice of primary variables. This is summarised in [variables]:

!table id=variables caption=Primary variables
| Phase state | Variable 1 | Variable 2 | Variable 3 |
| --- | --- | --- | --- |
| Single phase $(\alpha)$ | $P_{\alpha}$ | $T$ | $x_{\alpha}^{\kappa}$ |
| Two phases | $P_{\alpha}$ | $T$ | $S_{\alpha}$ |

There are two popular strategies to overcome this issue: primary variable switching, and
using a persistent set of primary variables.

## Primary variable switch

In this approach, the primary variables are switched depending on the phase state of the
model. For example, if only a single fluid phase is present, the primary variables used might
be the pressure, temperature and mass fraction of a component in the phase, see [variables]. If the phase state changes to a two-phase model, then the mass fraction
variable is *switched* to now represent the saturation of one of the fluid phases.

This approach has been used in several flow simulators, for example TOUGH2 [!citep](pruess1999)

## Persistent set of primary variables

An alternative approach is to use a set of primary variables that remain independent
in all phase states. This persistent primary variable approach has been implemented in
PorousFlow for miscible two-phase flow. In this approach, the primary variables are pressure of one phase $P_{\alpha}$,
temperature $T$ and total mass fraction of a fluid component summed over all phases $
\begin{equation}
z^{\kappa} = \frac{\sum_{\alpha} S_{\alpha} \rho_{\alpha} x_{\alpha}^{\kappa}}{\sum_{\alpha} S_{\alpha} \rho_{\alpha}}.
\end{equation}


!bibtex bibliography
