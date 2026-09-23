# InfiniteCylinderContactor

!syntax description /UserObjects/InfiniteCylinderContactor

## Description

Rigid contactor for an infinite circular cylinder of the given
[!param](/UserObjects/InfiniteCylinderContactor/radius), whose axis passes
through [!param](/UserObjects/InfiniteCylinderContactor/origin) in the
direction [!param](/UserObjects/InfiniteCylinderContactor/axis).  The axis
direction is normalized internally, so only its direction matters (any
nonzero vector works).

At a point $x$ the signed distance is the perpendicular distance to the
axis minus the radius,

!equation
g_{\text{LS}}(x) = \left| \left(I - \hat{a}\hat{a}^{T}\right)\,(x - o) \right| - R,

with $\hat{a}$ the unit axis, $o$ the axis origin, and $R$ the radius.
The convention matches every other [LevelSetContactor.md]: $g > 0$ outside
the cylinder, $g < 0$ inside, and the outward normal is $\hat{n} = d_\perp
/ |d_\perp|$ with $d_\perp$ the component of $(x - o)$ perpendicular to
$\hat{a}$.  The Hessian is $\left(P - \hat{n}\hat{n}^{T}\right) / |d_\perp|$
with $P = I - \hat{a}\hat{a}^{T}$; it captures the cylinder's curvature
in the tangential (circumferential) direction and is zero along the axis
and along the radial direction.

Queries on the axis itself ($|d_\perp| = 0$) are geometrically ambiguous
--- any radial direction is a valid normal.  The implementation returns a
zero vector for the normal and a zero Hessian there; downstream
[RigidBodyNodalNCPKernel.md] guards on $g > 0$ so a deformable node
sitting on the axis is an extreme configuration a well-posed input avoids.

The primary application is rolling-mill / roller-contact setups where the
roller can be treated as a straight, arbitrarily long circular cylinder;
see the
[rolling-reduction example](modules/contact/rigid_contact/examples/rolling_reduction.md)
for a two-pass thickness reduction driven by two of these contactors.

## Parameters

!syntax parameters /UserObjects/InfiniteCylinderContactor
