# Rank Two Scalar Tools

## Description

This is a set of functions to compute scalar
quantities such as invariants and components in specified directions from rank-2
tensors such as stress or strain. These functions are not directly invoked in the
input file, but are called by several other classes such as:
[RankTwoCylindricalComponent](/RankTwoCylindricalComponent.md),
[RankTwoDirectionalComponent](/RankTwoDirectionalComponent.md),
[RankTwoCartesianComponent](/RankTwoCartesianComponent.md),
[RankTwoInvariant](/RankTwoInvariant.md),
[RankTwoScalarAux](/RankTwoScalarAux.md),
and [RankTwoAux](/RankTwoAux.md).

The scalar quantities that can be computed include:

## Axial Stress

`AxialStress` calculates the scalar value of a Rank-2 tensor,
$T$, in the direction of the axis specified by the user.  The user should give
the starting point, $P^1$, and the end point, $P^2$ which define the axis.

\begin{equation}
\label{eq:axial_stress_scalar_type}
s = \hat{a}_i T_{ij} \hat{a}_j \quad \text{ where } \quad \hat{a}_i = \frac{P^2_i - P^1_i}{\left| P^2_i - P^1_i \right|}
\end{equation}
where $\hat{a}$ is the normalized direction vector for the axis defined by the points $P^1$ and $P^2$.

## Direction

`Direction` calculates the scalar value of a Rank-2 tensor, $T$,
in the direction selected by the user as shown by [eq:direction_scalar_type]:
\begin{equation}
\label{eq:direction_scalar_type}
s = D_i T_{ij} D_j
\end{equation}
where $D$ is the direction vector specified in the input file.


## Effective Strain Increment

Effective plastic strain or effective creep strain, which are computed as
integrals over the history of the inelastic strain as
\begin{equation}
s = \int_t\sqrt{\frac{2}{3} \dot{\epsilon}^p_{ij} \dot{\epsilon}^p_{ij}} \mathrm{d}t
\end{equation}
can be computed with the help of the `effectiveStrain` method.
The integration of the effective increment is performed in [RankTwoInvariant.md],
yielding the effective strain.

## Hoop Stress in Cylinderical System

`HoopStress` calculates the value of a Rank -2 tensor along the
hoop direction of a cylinder, shown in [eq:hoop_stress_scalar_type].  The
cylinder is defined with a normal vector from the current position to the
cylinder surface and a user specified axis of rotation.The user defines this
rotation axis with a starting point, $P^1$, and the end point, $P^2$.

\begin{equation}
\label{eq:hoop_stress_scalar_type}
s = \hat{n}^h_i T_{ij} \hat{n}^h_j
\end{equation}
where $\hat{n}^h$ is the hoop direction normal, defined as
\begin{equation}
\label{eq:hoop_direction_normal}
  \begin{aligned}
    \hat{n}^h & = \hat{n}^c \times \hat{a} \\
    \hat{n}^c & = \frac{P^c - \hat{n}^r}{\left| P^c - \hat{n}^r \right|} \\
    \hat{a} & = \frac{P^2 - P^1}{\left| P^2 - P^1 \right|}
  \end{aligned}
\end{equation}
where $P^c$ is the current sampling position point, and $\hat{n}^r$ is the direction normal to the plane defined by the cylinder axis of rotation vector and the direction normal to the axis of rotation at the current position $P^c$.

## Hoop Stress in Spherical System

<!-- // Given normal vector N=(n1,n2,n3) and current point C(c1,c2,c3), the tangential plane is then
// defined as n1(x-c1 + n2(y-c2) + n3(z-c3)=0. Let us assume n1!=0, the arbitrary point P on this
// plane can be taken as P(x,c2+r,c3+r) where r is the radius. The x can be solved as x =
// -r(n2+n3)/n1 + c1. The tangential vector PC is given as P-C. -->

`HoopStress` calculates the value of a Rank -2 tensor along the
tangential direction of a sphere, shown in [eq:hoop_stress_scalar_type_spherical].  The spherical system is defined by the center point $C(c_1,c_2,c_3)$. The radial direction $R(r_1,r_2,r_3)$ at current point $P(p_1,p_2,p_3)$ is calculated as $(P-C)$. The tangential plane at the Point $P$ is given as $r_1(x-p_1) + r_2(y-p_2) + r_3(z-p_3)=0$. Any vector that passes through $P$ on this plane is tangential to the spherical surface. To find a point $Q(q_1,q_2,q_3)$ on the tangential plane, we can freely set the values of two coordinates and the solve for last one using the equation of the plane. For example, we set $q_1=p_1+r$ and $q_2=p_2+r$ where $r$ is the norm of the radial direction vector. Then the $q_3$ is calculated as $q_3 = -(r_1+r_2)r/r_3+p_3$. The tangential vector $\hat{t}$ is defined as $Q-P$.

\begin{equation}
\label{eq:hoop_stress_scalar_type_spherical}
s = \hat{t}_i T_{ij} \hat{t}_j
\end{equation}


## Hydrostatic Stress

`Hydrostatic` calculates the hydrostatic scalar of a Rank-2
tensor, $T_{ij}$, as shown in [eq:hydrostatic_scalar_type].

\begin{equation}
\label{eq:hydrostatic_scalar_type}
s = \frac{Tr \left( T_{ij} \right)}{3} = \frac{T_{ii}}{3}
\end{equation}


## Invariant Values

### First Invariant

`FirstInvariant` calculates the first invariant of the specified
Rank-2 tensor, $T_{ij}$, according to [eq:first_invariant_scalar_type] from
[!cite](malvern1969introduction).
\begin{equation}
\label{eq:first_invariant_scalar_type}
I_T = Tr \left( T_{ij} \right) = T_{ii}
\end{equation}


### Second Invariant

`SecondInvariant` finds the second invariant of the
Rank-2 tensor, $T_{ij}$, as shown in [eq:second_invariant_scalar_type].  This
method is defined in [!cite](hjelmstad2007fundamentals).
\begin{equation}
\label{eq:second_invariant_scalar_type}
II_T = T_{ii} T_{jj} - \frac{1}{2} \left( T_{ij} T_{ij} + T_{ji} T_{ji} \right)
\end{equation}


### Third Invariant

`ThirdInvariant` computes the value of the Rank-2 tensor,
$T_ij$, third invariant as given in [eq:third_invariant_scalar_type] from
[!cite](malvern1969introduction).

\begin{equation}
\label{eq:third_invariant_scalar_type}
III_T = det \left( T_{ij} \right)  = \frac{1}{6} e_{ijk} e_{pqr} T_{ip} T_{jq} T_{kr}
\end{equation}
where $e$ is the Rank-3 permutation tensor.


## L2 Norm

`L2Norm` calculates the L2 normal of a Rank-2 tensor, $T_{ij}$,
as shown in [eq:l2_norm_scalar_type].

\begin{equation}
\label{eq:l2_norm_scalar_type}
s = \sqrt{T_{ij} T_{ij}}
\end{equation}

## Maximum Shear Stress

`MaxShear` calculates the maximum shear stress for a Rank-2
tensor, as shown in [eq:maxshear_scalar_type].
\begin{equation}
\label{eq:maxshear_scalar_type}
\sigma_{max}^{shear} = \frac{\sigma_{max}^{principal} - \sigma_{min}^{principal}}{2}
\end{equation}


## Principal Values

### Maximum Principal Quantity

`MaxPrincipal` calculates the largest principal value for a
symmetric tensor, using the calcEigenValues method from the Rank Two Tensor
utility class.


### Middle Principal Quantity

`MidPrincipal` finds the second largest principal
value for a symmetric tensor, using the calcEigenValues method from the Rank Two
Tensor utility class.


### Minimum Principal Quantity

`MinPrincipal` computes the smallest principal value for a
symmetric tensor, using the calcEigenValues method from the Rank Two Tensor
utility class.


## Radial Stress in Cylindrical System

`RadialStress` calculates the scalar component for a Rank-2
tensor, $T_{ij}$, in the direction of the normal vector from the user-defined
axis of rotation, as shown in [eq:radial_stress_scalar_type_cylindrical].
\begin{equation}
\label{eq:radial_stress_scalar_type_cylindrical}
s = \hat{n}^r_i T_{ij} \hat{n}^r_j
\end{equation}
where $\hat{n}^r$ is the direction normal to the plane defined by the cylinder axis of rotation
vector and the direction normal to the axis of rotation at the current position $P^c$.

## Radial Stress in Spherical System

`RadialStress` calculates the scalar component for a Rank-2
tensor, $T_{ij}$, in the direction of the normal vector from the user-defined
center point, as shown in [eq:radial_stress_scalar_type_spherical].
\begin{equation}
\label{eq:radial_stress_scalar_type_spherical}
s = \hat{n}^r_i T_{ij} \hat{n}^r_j
\end{equation}
where $\hat{n}^r$ is the direction defined by center point and current position $P^c.

## Stress Intensity

`StressIntensity` calculates the stress intensity for a Rank-2
tensor, as shown in [eq:sint_scalar_type].
\begin{equation}
\label{eq:sint_scalar_type}
s = 2 \sigma_{max}^{shear} = \sigma_{max}^{principal} - \sigma_{min}^{principal}
\end{equation}

This quantity is useful in evaluating whether a Tresca failure criteria has been
met and is two times the MaxShear quantity.


## Triaxiality Stress

`TriaxialityStress` finds the ratio of the hydrostatic measure,
$T_{hydrostatic}$, to the von Mises measure, $T_{vonMises}$, as shown in
[eq:triaxiality_scalar_type].  As the name suggests, this scalar measure is most
often used for stress tensors.
\begin{equation}
\label{eq:triaxiality_scalar_type}
s = \frac{T_{hydrostatic}}{T_{vonMises}} = \frac{\frac{1}{3} T_{ii}}{\sqrt{\frac{3}{2} S_{ij} S_{ij}}}
\end{equation}
where $S_{ij}$ is the deviatoric tensor of the Rank-2 tensor $T_{ij}$.

##Volumetric Strain

`VolumetricStrain` computes the volumetric strain, defined as
\begin{equation}
\label{eq:volumetric_strain}
\varepsilon_{vol} = \frac{\Delta V}{V}
\end{equation}
where $\Delta V$ is the change in volume and $V$ is the original volume.

This calculation assumes that the strains supplied as input($T$) are logarithmic
strains, which are by definition $log(L / L_0) $, where $L$ is the current
length and $L_0$ is the original length of a line segment in a given
direction.The ratio of the volume change of a strained cube to the original
volume is thus :
\begin{equation}
\label{eq:volumetric_strain_from_tensor}
s = \frac{\Delta V}{V} = \exp(T_{11}) * \exp(T_{22}) * \exp(T_{33}) - 1
\end{equation}
This is the value computed as the volumetric strain.


!alert! note title=Finite strain effects
This calculation assumes that the supplied Rank-2 tensor $T_{ij}$ is a
logarithmic strain, which is the strain quantity computed for finite strain
calculations. The small-strain equivalent of this calculation would be
\begin{equation}
\label{eq:volumetric_strain_small_strain}
s = T_{11} + T_{22} + T_{33}
\end{equation}
which assumes that engineering strains are supplied and ignores higher-order
terms. There is currently no option to compute this small-strain form of the
volumetric strain because at small strains, the differences between the finite
strain form used and the small strain approximation is small.
!alert-end!

## Von Mises Stress

`VonMisesStress` calculates the vonMises measure for a Rank-2
tensor, as shown in [eq:vonmises_scalar_type].  This quantity is usually applied
to the stress tensor.
\begin{equation}
\label{eq:vonmises_scalar_type}
s = \sqrt{\frac{3}{2} S_{ij} S_{ij}}
\end{equation}
where $S_{ij}$ is the deviatoric tensor of the Rank-2 tensor $T_{ij}$.
