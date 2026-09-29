# SCMFrictionChenTodreas

!syntax description /SCMClosures/SCMFrictionChenTodreas

## Overview

!! Intentional comment to provide extra spacing

This class is used to model the axial friction factor for a subchannel assembly with wire-wrapped/bare fuel pins in a triangular lattice or bare fuel pins in a quadrilateral lattice.

For triangular lattices, two Chen-Todreas friction-factor parameterizations are available:

- `Upgraded`, based on the upgraded Chen-Todreas correlation (UCTD) [!cite](todreas2021nuclear1), [!cite](chen2018upgraded);
- `Pacio`, based on the Pacio-Chen-Todreas parameterization (PCTD) [!cite](pacio2022analysis).

Equation and table numbers below refer to the UCTD and PCTD papers cited above.

The desired triangular-lattice parameterization is selected using the `friction_model` parameter. The default is `Upgraded`.

The two models use the same general form of the Cheng-Todreas detailed subchannel friction correlation but use different empirical coefficients for the flow-regime transition, wire-drag and wire-sweep terms, and intermittent-regime interpolation.

### Upgraded Chen-Todreas parameterization

For `friction_model = Upgraded`, the laminar and turbulent transition Reynolds numbers are given by Eqs. (3) and (2) of UCTD,

!equation
Re_L = 320 \times 10^{P/D - 1},

and

!equation
Re_T = 10^4 \times 10^{0.7(P/D - 1)} .

The turbulent wire-drag coefficient is evaluated as in Eq. (14) of UCTD, with the sign of the linear term corrected in [!cite](chen2018corrigendum),

!equation
W_{dT} =
\left[
19.56
- 98.71\left(\frac{D_w}{D}\right)
+ 303.47\left(\frac{D_w}{D}\right)^2
\right]
\left(\frac{H}{D}\right)^{-0.541},

with the laminar coefficient from Eq. (19) of UCTD,

!equation
W_{dL} = 1.4 W_{dT} .

The turbulent wire-sweep coefficient is given by Eq. (21) of UCTD,

!equation
W_{sT} =
-11 \log_{10}\left(\frac{H}{D}\right) + 19,

with Eq. (20) of UCTD,

!equation
W_{sL} = W_{sT} .

The intermittent-regime interpolation, Eq. (4) of UCTD, uses

!equation
\lambda = 7,
\qquad
\gamma = \frac{1}{3} .

### Pacio-Chen-Todreas parameterization

For `friction_model = Pacio`, the transition Reynolds numbers are independent of $P/D$ and are given by Eqs. (13) and (12) of PCTD with the coefficients of its Table 5,

!equation
Re_L = 700,
\qquad
Re_T = 10^4 .

The turbulent wire-drag coefficient is evaluated using Eq. (21) of PCTD as

!equation
W_{dT} =
\left[
15.2
- 48.0\left(\frac{D_w}{D}\right)
+ 148.6\left(\frac{D_w}{D}\right)^2
\right]
\left(\frac{H}{D}\right)^{-0.547},

with the laminar coefficient from Eq. (22) of PCTD,

!equation
W_{dL} = 0.8 W_{dT} .

The turbulent wire-sweep coefficient is given by Eq. (1) of PCTD,

!equation
W_{sT} =
-6.9 \log_{10}\left(\frac{H}{D}\right) + 12,

with Eq. (26) of PCTD,

!equation
W_{sL} = 1.2 W_{sT} .

The intermittent-regime interpolation, Eq. (35) of PCTD, uses

!equation
\lambda = 6.7,
\qquad
\gamma = 0.362 .

### Flow-regime interpolation

For both triangular-lattice parameterizations, the interpolation factor in the intermittent regime is evaluated using the bulk Reynolds number as in Eq. (5) of UCTD and Eq. (34) of PCTD,

!equation
\psi =
\frac{\ln(Re_b/Re_L)}
     {\ln(Re_T/Re_L)} .

The laminar and turbulent subchannel friction factors, Eqs. (A1) and (A2) of UCTD and Eqs. (16) and (15) of PCTD, are

!equation
f_L = C_{fL} Re^{-1},

and

!equation
f_T = C_{fT} Re^{-0.18},

where $Re$ is the local subchannel Reynolds number and $C_{fL}$ and $C_{fT}$ include the appropriate bare-pin and, when present, wire-wrap contributions.

The intermittent friction factor is calculated as in Eq. (4) of UCTD and Eq. (35) of PCTD,

!equation
f =
f_L (1-\psi)^\gamma
\left(1-\psi^\lambda\right)
+
f_T \psi^\gamma .

The values of $Re_L$, $Re_T$, $\lambda$, and $\gamma$ are determined by the selected `friction_model`. Table 5 of PCTD compares all coefficients of both parameterizations.

### Applicability

For triangular lattices, the closure flags a solution warning when $P/D$, wire-wrap $H/D$, number of pins, or the bulk Reynolds number $Re_b$ is outside the data range associated with the selected friction correlation. Both sets of ranges are the reported ranges of validity in Table 1 of PCTD.

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

For quadrilateral lattices, the existing Cheng-Todreas bare-pin friction-factor formulation is retained. The transition Reynolds numbers are given by Eqs. (3) and (2) of UCTD,

!equation
Re_L = 320 \times 10^{P/D - 1},
\qquad
Re_T = 10^4 \times 10^{0.7(P/D - 1)},

and the intermittent-regime interpolation, Eq. (4) of UCTD, uses

!equation
\lambda = 7,
\qquad
\gamma = \frac{1}{3} .

Plots of the friction factor versus Reynolds number for all SCM friction closures are given in [Friction Factor Closures Verification](subchannel/v&v/friction_factor_closures.md).

!syntax parameters /SCMClosures/SCMFrictionChenTodreas

!syntax inputs /SCMClosures/SCMFrictionChenTodreas

!syntax children /SCMClosures/SCMFrictionChenTodreas
