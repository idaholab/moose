# Relative permeability test descriptions

The details of the capillary pressure curve implementations can be found in [here](porous_flow/relative_permeability.md).


## Brooks-Corey

The [!cite](brookscorey1966) relative permeability model is an extension of the previous  [!cite](corey1954) formulation where the relative permeability of the wetting phase is given by
\begin{equation}
k_{\mathrm{r, w}} = \left(S_{\mathrm{eff}}\right)^{(2 + 3 \lambda)/\lambda},
\end{equation}
and the relative permeability of the non-wetting phase is
\begin{equation}
k_{\mathrm{r, nw}} = (1 - S_{\mathrm{eff}})^2 \left[1 - \left(S_{\mathrm{eff}}\right)^{(2 + \lambda)/\lambda}\right],
\end{equation}
where $\lambda$ is a user-defined exponent. When $\lambda = 2$, this formulation reduces
to the original [!cite](corey1954) form.

#### Test 1

The input file for this test:

!listing modules/porous_flow/test/tests/relperm/brooks_corey1.i

!media relperm/brook_corey1.png style=width:100%;margin-left:10px; caption=Brooks-Corey relative permeability Test case 1 id=brookcorey1

#### Test 2

The input file for this test:

!listing modules/porous_flow/test/tests/relperm/brooks_corey2.i

!media relperm/brook_corey2.png style=width:100%;margin-left:10px; caption=Brooks-Corey relative permeability Test case 2 id=brookcorey2

## Corey

[`PorousFlowRelativePermeabilityCorey`](/PorousFlowRelativePermeabilityCorey.md)

The relative permeability of the phase is given by [!cite](corey1954)
\begin{equation}
k_{\mathrm{r}} = S_{\mathrm{eff}}^{n},
\end{equation}
where $n$ is a user-defined quantity. Originally, [!cite](corey1954) used $n = 4$ for the wetting phase, but the PorousFlow module allows an arbitrary exponent to be used.

#### Test 1

The input file for this test:

!listing modules/porous_flow/test/tests/relperm/corey1.i

!media relperm/corey1.png style=width:100%;margin-left:10px; caption=Corey relative permeability Test case 1 id=corey1

#### Test 2

The input file for this test:

!listing modules/porous_flow/test/tests/relperm/corey2.i

!media relperm/corey2.png style=width:100%;margin-left:10px; caption=Corey relative permeability Test case 2 id=corey2

#### Test 3

The input file for this test:

!listing modules/porous_flow/test/tests/relperm/corey3.i

!media relperm/corey3.png style=width:100%;margin-left:10px; caption=Corey relative permeability Test case 3 id=corey3

#### Test 4

The input file for this test:

!listing modules/porous_flow/test/tests/relperm/corey4.i

!media relperm/corey4.png style=width:100%;margin-left:10px; caption=Corey relative permeability Test case 4 id=corey4


## van Genuchten

The relative permeability of the wetting phase is given by [!cite](vangenuchten1980)
\begin{equation}
k_{\mathrm{r}} = \sqrt{S_{\mathrm{eff}}} \left(1 - (1 -
S_{\mathrm{eff}}^{1/m})^{m} \right)^{2}.
\end{equation}
This has the numerical disadvantage that its derivative as
$S_{\mathrm{eff}}\rightarrow 1$ tends to infinity.  This means that
simulations where the saturation oscillates around
$S_{\mathrm{eff}}=1$ do not converge well.

Therefore, a *cut* version of the van Genuchten expression is also offered, which is
almost definitely indistinguishable experimentally from the original expression:
\begin{equation}
k_{\mathrm{r}} =
\begin{cases}
\textrm{van Genuchten} & \textrm{for } S_{\mathrm{eff}} < S_{c} \\
\textrm{cubic} & \textrm{for } S_{\mathrm{eff}} \geq S_{c}.
\end{cases}
\end{equation}
Here the cubic is chosen so that its value and derivative match the
van Genuchten expression at $S=S_{c}$, and so that it is unity at
$S_{\mathrm{eff}}=1$.

For the non-wetting phase, the van Genuchten expression is
\begin{equation}
k_{\mathrm{r}} = \sqrt{S_{\mathrm{eff}}} \left(1 - (1 -
S_{\mathrm{eff}})^{1/m} \right)^{2m} \ .
\end{equation}

#### Test 1

The input file for this test:

!listing modules/porous_flow/test/tests/relperm/vangenuchten1.i

!media relperm/vangen1.png style=width:100%;margin-left:10px; caption=van Genuchten relative permeability Test case 1 id=vangen1

#### Test 2

The input file for this test:

!listing modules/porous_flow/test/tests/relperm/vangenuchten2.i

!media relperm/vangen2.png style=width:100%;margin-left:10px; caption=van Genuchten relative permeability Test case 2 id=vangen2
