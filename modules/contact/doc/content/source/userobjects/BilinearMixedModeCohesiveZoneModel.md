# BilinearMixedModeCohesiveZoneModel

## Description

The `BilinearMixedModeCohesiveZoneModel` object computes a global traction vector
from a local traction vector that is calculated from a bilinear mixed mode cohesive
zone model. Theoretical equations and their application to particle fuel is described
and discussed in [!cite](jiang2021efficient).

This object implements the bilinear mixed mode traction separation law using the nodal-based mortar quantities
that are employed in the MOOSE formulation. Such quantities are identified with a tilde
at a secondary surface node $j$. The definitions of mortar surfaces and mortar quantities are identical to
those presented in previous technical reports ([!cite](martin2022implement,recuero2022mortar)),
which are in turn based on existing discretization approaches ([!cite](wohlmuth2011variationally,
puso2008segment)).

### Local interface frame

The cohesive zone model and [MortarGenericTraction](/MortarGenericTraction.md) run on the
undisplaced mesh, so the mortar segments pair each secondary point with the primary material point
it is bonded to, and this pairing does not change during the simulation. At secondary node $j$, the
local displacement jump is

\begin{equation}
  \tilde{\boldsymbol{\delta}}_{j} = \mathbf{Q}_{j}^{T} \left( \tilde{\mathbf{u}}_{j} +
  \Delta\tilde{\mathbf{X}}_{j} \right) - \mathbf{Q}_{0,j}^{T} \Delta\tilde{\mathbf{X}}_{j},
  \label{eq:local_jump}
\end{equation}

where $\tilde{\mathbf{u}}_{j}$ and $\Delta\tilde{\mathbf{X}}_{j}$ are the mortar-weighted
differences, primary minus secondary, of the displacement and of the reference position of the
paired points. $\Delta\tilde{\mathbf{X}}_{j}$ vanishes on a flat interface, whether or not the meshes
match, and is nonzero where the faceted primary and secondary surfaces do not coincide, as on a
curved interface with non-matching meshes. There it adds an error of the order of the in-plane
strain times the square of the element size to the jump under a deformation; a rigid motion leaves
the jump unchanged. The frames are

\begin{equation}
  \mathbf{Q}_{j} = \mathbf{E}_{j}(\mathbf{x}), \qquad \mathbf{Q}_{0,j} = \mathbf{E}_{j}(\mathbf{X}),
  \label{eq:current_frame}
\end{equation}

where $\mathbf{E}_{j}$ is an orthonormal frame built from the secondary faces that share node $j$,
evaluated at the current positions $\mathbf{x}$ or at the reference positions $\mathbf{X}$. Its
first column is the averaged nodal normal of these faces, as in mortar contact
[!citep](puso2004mortar,popp2009finite). In 2D the tangent is the normal rotated by $+90$ degrees.
In 3D the reference tangents are carried along by the rotation part of the surface deformation
gradient at node $j$ (the polar decomposition of the in-plane stretch, averaged over the faces that
share the node), so stretching the surface does not turn them. Because both frames come from the
same face average, $\mathbf{Q}_{j} = \mathbf{Q}_{0,j}$ in the reference configuration, also on a
faceted surface.

If a rigid motion with rotation $\mathbf{R}$ is superposed on any state of the bonded bodies, the
weighted relative position $\tilde{\mathbf{u}}_{j} + \Delta\tilde{\mathbf{X}}_{j}$ and the frame
$\mathbf{Q}_{j}$ are both rotated by $\mathbf{R}$, so $\tilde{\boldsymbol{\delta}}_{j}$ in
[eq:local_jump] does not change. In particular, a rigid motion of the reference configuration gives
$\mathbf{Q}_{j} = \mathbf{R} \mathbf{Q}_{0,j}$ and $\tilde{\boldsymbol{\delta}}_{j} = \mathbf{0}$. This holds on any mesh, including curved and
non-matching interfaces, and is the kinematic part of the frame indifference required of cohesive
models at finite displacements [!citep](mosler2011thermodynamically).

The local traction $\boldsymbol{\tau}_{j}$ from the constitutive law below is mapped to global
coordinates with the same frame, $\mathbf{t}_{j} = \mathbf{Q}_{j} \boldsymbol{\tau}_{j}$, interpolated
with the secondary shape functions and integrated over the undisplaced mortar segments. The
traction is therefore a force per unit reference area (a nominal traction), and the strengths and
the fracture energies $G_{Ic}$ and $G_{IIc}$ are per unit reference area. This follows finite
deformation interface formulations that integrate a nominal, first Piola-Kirchhoff traction over
the undeformed interface [!citep](reinoso2014consistent, vandenbosch2008development), and in which
the interface energy is a density per unit reference area [!citep](mosler2011thermodynamically). It
describes an interface with a fixed number of bonds per unit material area, so stretching the
interface in its plane does not change the energy needed to separate a material patch. The
interface kernel cohesive zone models of the solid mechanics module and earlier versions of this
model define the traction per unit current area instead, so under an in-plane stretch their
strengths and fracture energies differ from this model's by the ratio of current to reference
interface area. The derivatives of $\mathbf{Q}_{j}$ with respect
to the displacements are included through automatic differentiation, so the Jacobian is exact. A
folded secondary surface, for which the tangents are undefined, or a non-finite frame, jump, or
traction cuts the time step.

The frame derivatives span the incident-face star of each secondary node, so the frame needs more
AD derivative capacity than the jump alone. The requirement depends on the secondary face order,
the number of faces sharing a node, the number of displacement variables and the pairing with the
primary faces, so no single derivative size suits every problem. 2D interfaces and small 3D
interfaces run with the default capacity; larger or second-order 3D interfaces may not. If the
capacity is insufficient, reconfigure with a larger `--with-derivative-size` value and rebuild the
MOOSE libraries and application consistently.

The nodal jump and the nodal strengths are mortar integrals divided by the integral of the nodal
test function over the mortar segments. Two conditions keep that integral positive:

- Over a whole face, the standard basis integrates to zero at second-order TRI6 vertices and to a
  negative value at second-order QUAD8 corners, so those faces are rejected with an error. With
  [!param](/UserObjects/BilinearMixedModeCohesiveZoneModel/use_dual) set to `true`, the mortar
  integrals are weighted, and the traction is interpolated, with the dual basis of the displacement
  shape functions instead. On TRI6 and QUAD8 faces this is the transformed dual basis
  [!citep](popp2012dual), whose nodal integrals over a whole face are positive. The displacements
  keep their standard shape functions.
- The mortar segments cover only the part of a secondary face that pairs with the primary surface.
  Second-order test functions, standard and dual alike, are negative on part of a face, so the
  integral of a vertex function over a partly covered face can be zero or negative, which makes
  the jump meaningless. A second-order secondary face that is only partly covered is therefore
  rejected with an error. First-order test functions are nonnegative, so partly covered
  first-order faces are supported. Dual mortar methods resolve partial coverage for first-order
  interpolation [!citep](popp2013improved), and extending them to second-order interpolation
  remains open, because the quadratic shape functions are not positive.

### Quadratic failure criterion

The softening of the interface in the bilinear mixed mode traction separation law is given by a quadratic failure criterion. The onset of softening of the interface takes place when the traction state reaches the following equality:
\begin{equation}
    {\left(\frac{<{\tilde{\sigma}_{N,j}}>}{N}\right)}^{2} + {\left(\frac{{\tilde{\tau}_{1,j}}}{T_{1}}\right)}^{2} + {\left(\frac{{\tilde{\tau}_{2,j}}}{T_{2}}\right)}^{2} = 1.0,
    \label{eq:tsl_softening}
\end{equation}
where $<{\tilde{\sigma}_{N,j}}>$, $\tilde{\tau}_{1,j}$, and $\tilde{\tau}_{2,j}$ are the (positive) normal traction, the shear traction along direction one, and the shear traction along direction two, respectively. These three traction components are computed at each node $j$ using interpolated, weighted quantities. $N$, $T_{1}$, and $T_{2}$ are the interlaminar tensile and shear strengths. The model takes one
shear strength, [!param](/UserObjects/BilinearMixedModeCohesiveZoneModel/shear_strength), so
$T_{1} = T_{2}$ and the tangential response does not depend on the in-plane direction, which the
tangential frame of a deformed 3D interface requires.

### Mode mixity

The displacement on the interface can be split into normal and tangential parts. Therefore, the mixed mode relative displacement can be expressed as:
\begin{equation}
  \tilde{\delta}_{m,j} = \sqrt{\tilde{\delta}_{t1,j}^{2} + \tilde{\delta}_{t2,j}^{2} + \langle{\tilde{\delta}_{N,j}\rangle}^{2}},
  \label{eq:mixed_disp}
\end{equation}
where subscripts $N$, and $t1$ and $t2$ denote the normal direction and the two tangential directions, respectively.

The formulation assumes that the interface traction generated by the displacements is proportional; in essence, a traction component is equal to a penalty factor times the displacement jump component ($\tau_{i} = K_{i}\delta_{i}$, where $K_{i}$ is the penalty factor). If $\tilde{\delta}_{N,j} > 0$, the mixity ratio is obtained as:
\begin{equation}
  \tilde{\beta} = \frac{\sqrt{\tilde{\delta}_{t1,j}^{2} + \tilde{\delta}_{t2,j}^{2}}}{\tilde{\delta}_{N,j}}.
  \label{eq:mixity_ratio}
\end{equation}

Assuming $T_1 = T_2$, the uni-mode relative displacements at the beginning of softening (denoted by superscript ``0'') are given by:
\begin{equation}
  \tilde{\delta}_{ti,j}^{0} = \frac{T_{i}}{K_{i}},\,  \tilde{\delta}_{N,j}^{0} = \frac{N}{K_{i}}.
  \label{eq:rel_disp}
\end{equation}

The mixed mode relative displacement corresponding to the initiation of softening, $\tilde{\delta}_{m,j}^{0}$, is defined by the following multi-part definition:
\begin{equation}
\tilde{\delta}_{m,j}^{0} =
\left\{
 \begin{array}{ll}
  \tilde{\delta}_{N,j}^{0} \tilde{\delta}_{t2,j}^{0} \sqrt{\frac{1 + \tilde{\beta}^{2}}{\tilde{(\delta}_{t2,j}^{0})^2 + (\tilde{\beta} \tilde{\delta}_{N,j}^{0})^2}}  &  {if } \tilde{\delta}_{N,j}^{0} \geq 0, \\
  \tilde{\delta}_{t2,j}^{0}  &  {if } \tilde{\delta}_{N,j}^{0}  < 0.
 \end{array}
\right.
    \label{eq:mix_mode_rel_disp}
\end{equation}

### Mode mixity Delamination propagation prediction

The new CZM mortar implementation also accounts for two types of delamination propagation models. (1) A power law-based criterion and (2) The Benzeggagh-Kenane (BK) criterion (see, e.g.,[!cite](benzeggagh1996measurement,bui2011modified)).

#### Power law-based delamination criterion

The power law criterion may be described as:
\begin{equation}
\left(\frac{G_{I}}{G_{IC}}\right)^{\alpha} + \left(\frac{G_{II}}{G_{IIC}}\right)^{\alpha} = 1.0,
\label{eq:power_law_crit}
\end{equation}
where $G_{I}$ and $G_{II}$ are the critical energy release rates for mixed modes; whereas $G_{IC}$ and $G_{IIC}$ are the critical energy release rates for pure modes. The power exponent $\alpha$ is can be obtained experimentally.

During the delamination or debonding process, a relative interface displacement may be reached such that cohesive fores completely disappear, i.e. the cohesive model ceases to influence boundary tractions. Such a mixed-mode displacement value is defined as follows (note that we carry the node-wise definition, $j$):
\begin{equation}
\tilde{\delta}_{m,j}^{f} =
\left\{
 \begin{array}{ll}
  \frac{2(1+\tilde{\beta}^{2})}{K \tilde{\delta}_{m,j}^{0}} \left[\left( \frac{1}{G_{IC}}\right)^{\alpha} + \left( \frac{\tilde{\beta}^{2}}{G_{IIC}}\right)^{\alpha}\right]^{\frac{-1}{\alpha}}   &  {if } \tilde{\delta}_{N,j}^{0} \geq 0, \\
  \sqrt{\tilde{\delta}_{1,j}^{f} + \tilde{\delta}_{2,j}^{f}}  &  {if } \tilde{\delta}_{N,j}^{0} < 0.
 \end{array}
\right.
    \label{eq:mix_mode_decohesion}
\end{equation}

#### Benzeggagh-Kenane (BK) delamination criterion

The BK criterion can be written as ([!cite](benzeggagh1996measurement)):
\begin{equation}
G_{TC} = G_{IC} + (G_{IIC} - G_{IC}) (\frac{G_{II}}{G_{T}})^{\eta},
\label{eq:bk_criterion}
\end{equation}
where $G_{T} = G_{I} + G_{II}$, $G_{TC}$ is the total critical strain energy release rate, and $\eta$ is a model parameter.

The mixed-mode total decohesion displacements are, for the BK criterion:
\begin{equation}
\tilde{\delta}_{m,j}^{f} =
\left\{
 \begin{array}{ll}
  \frac{2}{K \tilde{\delta}_{m,j}^{0}} \left[ G_{IC} + \left(G_{IIC} - G_{IC}\left[\frac{\tilde{\beta}^{2}}{1 + \tilde{\beta}^{2}}\right]^{\eta}\right)\right]   &  {if } \tilde{\delta}_{N,j}^{0} \geq 0, \\
  \sqrt{\tilde{\delta}_{1,j}^{f} + \tilde{\delta}_{2,j}^{f}}  &  {if } \tilde{\delta}_{N,j}^{0} < 0.
 \end{array}
\right.
    \label{eq:bk_mix_mode_decohesion}
\end{equation}

### Constitutive model

The CZM constitutive models relate the weighted interface jump components to the interface traction in the local frame of reference. The traction, in the local frame of reference, can be expressed as:
\begin{equation}
    \tau_{l} = D_{li} \tilde{\delta}_{i},
\label{eq:traction}
\end{equation}
where $\tau_{l}$ is the $l$ component of the local traction and $D_{li}$ is an effective stiffness. The constitutive components are computed as:
\begin{equation}
 \tau_{l} =
\left\{
 \begin{array}{ll}
  (1 - \omega) K \tilde{\delta}_{\text{active}}  &  {if } \tilde{\delta}_{N,j}^{0} \geq 0, \\
  K \tilde{\delta}_{\text{inactive}}  &  {if } \tilde{\delta}_{N,j}^{0} < 0,
 \end{array}
\right.
\label{eq:constitutive}
\end{equation}
where $\tilde{\delta}_{\text{active}}$ represents the CZM weighted interface jump with shear components, i.e. it applies shear stiffness, and normal stiffness only if a gap exists. $\tilde{\delta}_{\text{inactive}}$ applies a normal traction that prevents interpenetration. A few notes about the previous equation:

- The action of $\delta_{\text{inactive}}$ can potentially be replaced with (mortar) mechanical contact enforcement. \\
- The local traction $\tau_{l}$ is computed nodally with weighted quantities, but is interpolated and applied on the primary and secondary surfaces using Lagrange interpolation functions. \\
- The constitutive model can be expanded as needed in the user object.

The damage, $\omega$, is computed as follows (we omit the nodal subscript):
\begin{equation}
\omega = \frac{\tilde{\delta}_{m}^{f} \left( \tilde{\delta}_{m}^{\text{max}} - \tilde{\delta}_{m}^{0}\right)}{\tilde{\delta}_{m}^{\text{max}} \left( \tilde{\delta}_{m}^{f} - \tilde{\delta}_{m}^{0}\right)}.
\end{equation}

### Auxiliary output conventions

The local-frame displacement jump and cohesive traction components can be output with
[CohesiveZoneMortarUserObjectAux](/CohesiveZoneMortarUserObjectAux.md). The tangential
displacement jump effective value is the tangential magnitude, `sqrt(t1^2 + t2^2)`.
The cohesive traction tangential magnitude uses the same tangential norm, while the cohesive
traction effective value is the full local traction norm, `sqrt(tn^2 + t1^2 + t2^2)`. In two dimensions, the second tangential component, `t2`, is zero.

The traction components in the local coordinate system follow the convention used internally
in the model, which is that opening is positive and tensile traction is negative.

### Viscous regularization

Sudden degradation of stiffness may be give rise to the appearance of large displacements. To alleviate this problematic numerical behavior in quasti-static simulations, we include in the constitutive equations a viscous damage $\omega_{v}$ instead:
\begin{equation}
\dot{\omega}_{v} = \frac{1}{\mu} \left( \omega - \omega_v\right).
\end{equation}

### Use standard mortar contact to enforce interpenetratation

The standard cohesive zone model uses a penalty method to prevent penetration in compression. Alternatively, users may choose to use Mortar contact to enforce the normal constraint. To do this, set [!param](/UserObjects/BilinearMixedModeCohesiveZoneModel/set_compressive_traction_to_zero) to `true` to remove the compressive traction from the cohesive zone model, and add a [PenaltyWeightedGapUserObject](/PenaltyWeightedGapUserObject.md) on the displaced mesh, used by [NormalMortarMechanicalContact](/NormalMortarMechanicalContact.md), to handle normal enforcement.

### Friction between debonded surfaces

The cohesive zone model does not compute contact or friction forces. For frictional contact, use a [PenaltyFrictionUserObject](/PenaltyFrictionUserObject.md) instead of the [PenaltyWeightedGapUserObject](/PenaltyWeightedGapUserObject.md), and add [TangentialMortarMechanicalContact](/TangentialMortarMechanicalContact.md) constraints that use it. The friction it computes does not depend on the cohesive damage.

Example of usage:

!listing modules/contact/test/tests/cohesive_zone_model/mortar_czm.i block=UserObjects/czm_uo

A cohesive zone model combined with frictional contact:

!listing modules/contact/test/tests/cohesive_zone_model/cohesive_friction.i block=UserObjects

!syntax parameters /UserObjects/BilinearMixedModeCohesiveZoneModel

!syntax inputs /UserObjects/BilinearMixedModeCohesiveZoneModel

!syntax children /UserObjects/BilinearMixedModeCohesiveZoneModel
