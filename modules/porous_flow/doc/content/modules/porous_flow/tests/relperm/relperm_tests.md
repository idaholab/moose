# Relative permeability test descriptions

#### The detail documentation of the implementation of this code packet can be founded in [here](porous_flow/relative_permeability.md).


## Brooks-Corey

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

#### Test 1

The input file for this test:

!listing modules/porous_flow/test/tests/relperm/vangenuchten1.i

!media relperm/vangen1.png style=width:100%;margin-left:10px; caption=van Genuchten relative permeability Test case 1 id=vangen1

#### Test 2

The input file for this test:

!listing modules/porous_flow/test/tests/relperm/vangenuchten2.i

!media relperm/vangen2.png style=width:100%;margin-left:10px; caption=van Genuchten relative permeability Test case 2 id=vangen2
