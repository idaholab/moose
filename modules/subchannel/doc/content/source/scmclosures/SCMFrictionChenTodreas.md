# SCMFrictionChenTodreas

!syntax description /SCMClosures/SCMFrictionChenTodreas

## Overview

!! Intentional comment to provide extra spacing

This class is used to model the axial friction factor for a subchannel assembly with wire-wrapped/bare fuel pins in a triangular lattice or bare fuel pins in a quadrilateral lattice.

For triangular lattices, two Cheng-Todreas friction-factor parameterizations are available:

- `Upgraded`, based on the upgraded Cheng-Todreas correlation [!cite](todreas2021nuclear1), [!cite](chen2018upgraded);
- `Pacio`, based on the Pacio-Chen-Todreas parameterization [!cite](pacio2022analysis).

The desired triangular-lattice parameterization is selected using the `friction_model` parameter. The default is `Upgraded`.

The two models use the same general form of the Cheng-Todreas detailed subchannel friction correlation but use different empirical coefficients for the flow-regime transition, wire-drag and wire-sweep terms, and intermittent-regime interpolation.

### Upgraded Cheng-Todreas parameterization

For `friction_model = Upgraded`, the laminar and turbulent transition Reynolds numbers are

!equation
Re_L = 320 \times 10^{P/D - 1},

and

!equation
Re_T = 10^4 \times 10^{0.7(P/D - 1)} .

The turbulent wire-drag coefficient is evaluated as

!equation
W_{dT} =
\left[
19.56
- 98.71\left(\frac{D_w}{D}\right)
+ 303.47\left(\frac{D_w}{D}\right)^2
\right]
\left(\frac{H}{D}\right)^{-0.541},

with the laminar coefficient

!equation
W_{dL} = 1.4 W_{dT} .

The turbulent wire-sweep coefficient is

!equation
W_{sT} =
-11 \log_{10}\left(\frac{H}{D}\right) + 19,

with

!equation
W_{sL} = W_{sT} .

The intermittent-regime interpolation uses

!equation
\lambda = 7,
\qquad
\gamma = \frac{1}{3} .

### Pacio-Chen-Todreas parameterization

For `friction_model = Pacio`, the transition Reynolds numbers are independent of $P/D$ and are given by

!equation
Re_L = 700,
\qquad
Re_T = 10^4 .

The turbulent wire-drag coefficient is evaluated as

!equation
W_{dT} =
\left[
15.2
- 48.0\left(\frac{D_w}{D}\right)
+ 148.6\left(\frac{D_w}{D}\right)^2
\right]
\left(\frac{H}{D}\right)^{-0.547},

with the laminar coefficient

!equation
W_{dL} = 0.8 W_{dT} .

The turbulent wire-sweep coefficient is

!equation
W_{sT} =
-6.9 \log_{10}\left(\frac{H}{D}\right) + 12,

with

!equation
W_{sL} = 1.2 W_{sT} .

The intermittent-regime interpolation uses

!equation
\lambda = 6.7,
\qquad
\gamma = 0.362 .

### Flow-regime interpolation

For both triangular-lattice parameterizations, the interpolation factor in the intermittent regime is evaluated using the bulk Reynolds number as

!equation
\psi =
\frac{\ln(Re_b/Re_L)}
     {\ln(Re_T/Re_L)} .

The laminar and turbulent subchannel friction factors are

!equation
f_L = C_{fL} Re^{-1},

and

!equation
f_T = C_{fT} Re^{-0.18},

where $Re$ is the local subchannel Reynolds number and $C_{fL}$ and $C_{fT}$ include the appropriate bare-pin and, when present, wire-wrap contributions.

The intermittent friction factor is calculated as

!equation
f =
f_L (1-\psi)^\gamma
\left(1-\psi^\lambda\right)
+
f_T \psi^\gamma .

The values of $Re_L$, $Re_T$, $\lambda$, and $\gamma$ are determined by the selected `friction_model`.

### Applicability

For triangular lattices, the closure flags a solution warning when $P/D$, wire-wrap $H/D$, number of pins, or the bulk Reynolds number $Re_b$ is outside the data range associated with the selected friction correlation.

For the `Upgraded` parameterization, the implemented applicability ranges are

!equation
1.0 \leq P/D \leq 1.42,

!equation
8.0 \leq H/D \leq 52.0,

!equation
7 \leq N_{\mathrm{pin}} \leq 217,

and

!equation
50 \leq Re_b \leq 10^6 .

For the `Pacio` parameterization, the implemented applicability ranges are

!equation
1.02 \leq P/D \leq 1.42,

!equation
7.5 \leq H/D \leq 54.0,

!equation
19 \leq N_{\mathrm{pin}} \leq 217,

and

!equation
10 \leq Re_b \leq 3\times10^5 .

### Quadrilateral lattices

The `friction_model` selection applies only to triangular lattices.

For quadrilateral lattices, the existing Cheng-Todreas bare-pin friction-factor formulation is retained. The transition Reynolds numbers are

!equation
Re_L = 320 \times 10^{P/D - 1},
\qquad
Re_T = 10^4 \times 10^{0.7(P/D - 1)},

and the intermittent-regime interpolation uses

!equation
\lambda = 7,
\qquad
\gamma = \frac{1}{3} .

!syntax parameters /SCMClosures/SCMFrictionChenTodreas

!syntax inputs /SCMClosures/SCMFrictionChenTodreas

!syntax children /SCMClosures/SCMFrictionChenTodreas
