# SCMMixingChenTodreas

!syntax description /SCMClosures/SCMMixingChenTodreas

## Overview

!! Intentional comment to provide extra spacing

This closure class is used to model the turbulent mixing coefficient $\beta$ using the Cheng-Todreas correlations for triangular assemblies with wire-wrapped pins. Two mixing-model parameterizations are available: the original Cheng-Todreas (1986) model and the Pacio-Chen-Todreas model.

The implementation is based on:

- Hydrodynamic models and correlations for bare and wire-wrapped hexagonal rod bundles—bundle friction factors, subchannel friction factors and mixing parameters, Cheng and Todreas [!cite](cheng1986hydrodynamic).

- The Pacio-Chen-Todreas formulation [!cite](pacio2022analysis), which introduces an updated mixing treatment to improve the prediction of flow redistribution between interior, edge, and corner subchannels.

The desired parameterization is selected using the `mixing_model` parameter:

- `1986` uses the original Cheng-Todreas mixing correlation. In addition it calculates the sweep coefficient for the periphery gaps.
- `Pacio` retains the original Cheng-Todreas treatment where applicable and applies the Pacio mixing formulation to center-edge and edge-corner interfaces.

### Cheng-Todreas (1986) mixing

For the `1986` model, the base turbulent mixing parameter is applied to gaps connected to a center subchannel. The mixing parameter is used in the global turbulent crossflow relation

!equation
w'_{ij} = \beta S_{ij} \bar{G} .

The laminar and turbulent mixing coefficients are calculated using the original Cheng-Todreas formulation and the center-subchannel projected wire area $A_{r1}$ and bare flow area $A'_1$.

The flow-regime transition limits are

!equation
Re_L = 320 \times 10^{P/D - 1},

and

!equation
Re_T = 10^4 \times 10^{0.7(P/D - 1)} .

In the intermittent regime, the interpolation factor is

!equation
\psi =
\frac{\ln(Re/Re_L)}
     {\ln(Re_T/Re_L)} ,

and the mixing coefficient is interpolated according to

!equation
C_m = C_{mL} + \left(C_{mT}-C_{mL}\right)\psi^{2/3} .

The resulting turbulent mixing parameter is

!equation
\beta =
C_m
\left(\frac{A_{r1}}{A'_1}\right)^{1/2}
\tan\theta .

### Pacio-Chen-Todreas mixing

When `mixing_model = Pacio` is selected, the Pacio formulation is applied specifically to interfaces involving the edge subchannel:

- center-edge interfaces use the Pacio mixing formulation;
- edge-corner interfaces use the Pacio mixing formulation;
- center-center interfaces retain the original Cheng-Todreas (1986) mixing formulation.
- sweep-flow implementation remains unchanged.

Thus, selecting `Pacio` does not globally replace the original Cheng-Todreas mixing model. Instead, it replaces or supplements the mixing treatment at the interfaces for which the Pacio formulation is applied.

The Pacio mixing coefficient depends on the local flow-split parameters

!equation
X_i = \frac{V_i}{\bar{V}},
\qquad
X_j = \frac{V_j}{\bar{V}},

where $V_i$ and $V_j$ are the axial velocities of the neighboring subchannels and $\bar{V}$ is the bundle bulk velocity.

For the turbulent regime, the flow-split dependence is evaluated from

!equation
W_{mT} =
\frac{8.8}{Re^{0.18}}
\frac{X_i^{\,2-0.18}-X_j^{\,2-0.18}}
     {X_i-X_j} .

When $X_i = X_j$, the limiting value of the fractional term is used to avoid division by zero:

!equation
\lim_{X_j\rightarrow X_i}
\frac{X_i^{1.82}-X_j^{1.82}}
     {X_i-X_j}
=
1.82 X_i^{0.82} .

For the Pacio parameterization, the transition limits are

!equation
Re_L = 700,
\qquad
Re_T = 10^4 .

The laminar mixing coefficient is

!equation
W_{mL} = 0,

and in the intermittent regime the mixing coefficient is interpolated as

!equation
C_m =
W_{mL} +
\left(W_{mT}-W_{mL}\right)\psi^{2/3},

where

!equation
\psi =
\frac{\ln(Re/Re_L)}
     {\ln(Re_T/Re_L)} .

For both center-edge and edge-corner interfaces, the Pacio mixing parameter uses the edge-subchannel projected wire area $A_{r2}$ and bare flow area $A'_2$:

!equation
\beta =
C_m
\left(\frac{A_{r2}}{A'_2}\right)^{1/2}
\tan\theta .

The Pacio mixing treatment is therefore applied according to the following interface behavior:

| Interface | `1986` | `Pacio` |
| :- | :- | :- |
| center-center | Cheng-Todreas (1986) | Cheng-Todreas (1986) |
| center-edge | Cheng-Todreas (1986) | Pacio-Chen-Todreas |
| edge-edge | none | none |
| edge-corner | none | Pacio-Chen-Todreas |
| corner-corner | none | none |

### Peripheral sweep flow

The Cheng-Todreas peripheral sweep-flow model is treated separately from the base turbulent mixing parameter.

For edge and corner gaps, `computeSweepFlowMixingParameter()` returns the sweep-flow parameter used by the triangular-assembly enthalpy equation. This contribution is not added to the global turbulent crossflow parameter which in the `Cheng-Todreas (1986)` case is zero for the periphery gaps.

The peripheral sweep-flow correlation always uses the original `Cheng-Todreas (1986)` formulation, including when `mixing_model = Pacio` is selected. The Pacio parameterization modifies only the turbulent mixing treatment described above and does not replace the Cheng-Todreas peripheral sweep-flow correlation.

The sweep-flow transition limits therefore remain

!equation
Re_L = 320 \times 10^{P/D - 1},
\qquad
Re_T = 10^4 \times 10^{0.7(P/D - 1)}

for both values of `mixing_model`.

The sweep-flow parameter uses the edge-subchannel projected wire area $A_{r2}$ and bare flow area $A'_2$.

The sweep-flow parameter is calculated as

!equation
\beta_s =
C_s
\left(\frac{A_{r2}}{A'_2}\right)^{1/2}
\tan\theta ,

where $C_s$ is the Cheng-Todreas sweep-flow coefficient evaluated according to the flow regime.

### Applicability

The closure flags a solution warning when $P/D$, $H/D$, the number of pins, or the bulk Reynolds number is outside the data range associated with the selected mixing correlation.

Information about the use of $\beta$ can be found in [Turbulent crossflow](subchannel_theory.md#turbulent-crossflow).

Additionally, the user may opt to provide the turbulent momentum mixing parameter `CT`. Information about the use of this parameter can be found in [Turbulent momentum transfer](subchannel_theory.md#turbulent-momentum-transfer).

!syntax parameters /SCMClosures/SCMMixingChenTodreas

!syntax inputs /SCMClosures/SCMMixingChenTodreas

!syntax children /SCMClosures/SCMMixingChenTodreas
