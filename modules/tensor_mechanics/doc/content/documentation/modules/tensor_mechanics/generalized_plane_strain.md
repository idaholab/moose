# Generalized Plane Strain

## Description

The generalized plane strain problem has found use in many important applications. Bearing the same name as generalized plane strain problem, different presentations have been found in the literature. The simplest one is an extension of the plane strain problem by allowing a non-vanishing constant direction strain in the out-of-plane direction [citep:Adams1967gps]. This has been further generalized by allowing two rotations degrees of freedom [citep:Abaqus2014gps]. An even more generalized form includes the anticlastic problem associated with the out-of-plane shear [citep:Adams1984gps, Li1999gps]. In [tensor mechanics module](tensor_mechanics/index.md), the simplest form was implemented, i.e. an extra degree of freedom representing the out-of-plane direct strain is included in addition to the conventional plane strain problem [citep:Li2005gps].

## Formulation

The deformation of a cylindrical body can be treated as a generalized plane strain problem if it is free from body force and surface traction in the longitudinal direction, while all loads, including body forces and surface traction, are properly balanced in any transverse plane and remain constant along the longitudinal direction. For the description of the problem, it is convenient to introduce the coordinate system so that the $x$-$y$ plane is positioned transverse to the cylindrical body and the $z$-axis runs in the longitudinal direction. Under the conditions described, the in-plane stresses are not functions of $z$. Implied is that the in-plane strains are not functions of $z$ from the constitutive relationship for linearly elastic and homogeneous materials. The generalized plane strain problem is 2D in presentation, defined in a 2D domain $A$ in the $x$-$y$ plane, which is the cross-section of the cylindrical body.

The kinematical equations of the generalized plane strain problem are identical to those for the plane stress or strain problems, given as
\begin{equation}
\left\{\begin{matrix}
\epsilon_{xx} \\
\epsilon_{yy} \\
\gamma_{xy}
\end{matrix}\right\}
=
\left\{\begin{matrix}
\frac{\partial u}{\partial x} \\
\frac{\partial v}{\partial y} \\
\frac{\partial v}{\partial x} + \frac{\partial u}{\partial y}
\end{matrix}\right\}
\end{equation}

The equilibrium equations for the generalized plane strain problem in the $x$-$y$ plane are given as
\begin{equation}
\begin{matrix}
\frac{\partial \sigma_{xx}}{\partial x} + \frac{\partial \sigma_{xy}}{\partial y} + \bar{f}_{x} = 0 \\
\frac{\partial \sigma_{xy}}{\partial x} + \frac{\partial \sigma_{yy}}{\partial y} + \bar{f}_{y} = 0
\end{matrix}
\end{equation}
in $A$ where $\left\{ \bar{f} \right\}$ are the body forces.

A further condition is required associated with the ends of the cylindrical body, i.e. planes perpendicular to the longitudinal axis. If a constant strain is prescribed as the deformation compatibility condition in the $z$-direction
\begin{equation}
\epsilon_{zz} = \bar{\epsilon}_{zz}
\end{equation}
Alternatively, a force as the stress resultant $\bar{N}_{zz}$ in the $z$-direction can be prescribed, the condition is the equilibrium condition in $z$-direction, given as
\begin{equation}
N_{zz} = \int_{A}{\sigma_{zz}dA} = \bar{N}_{zz}
\end{equation}
The stress resultant $N_{zz}$ conjugates with constant strain $\epsilon_{zz}$.

The constitutive equations in terms of stress-strain relationship are given as
\begin{equation}
\left\{ \begin{matrix}
\sigma_{xx} \\
\sigma_{yy} \\
\sigma_{zz} \\
\tau_{xy}
\end{matrix} \right\}
=
\left[ \begin{matrix}
C_{11} & C_{12} & C_{13} & 0 \\
C_{12} & C_{22} & C_{23} & 0 \\
C_{13} & C_{23} & C_{33} & 0 \\
0 & 0 & 0 & C_{66}
\end{matrix} \right]
\left( \left\{ \begin{matrix}
\epsilon_{xx} \\
\epsilon_{yy} \\
\epsilon_{zz} \\
\gamma_{xy}
\end{matrix} \right\}
-
\left\{ \begin{matrix}
\alpha_{x} \\
\alpha_{y} \\
\alpha_{z} \\
0
\end{matrix} \right\} \Delta T \right)
\end{equation}
where $C_{ij}$ are material's stiffness coefficients using Voigt notation, $\alpha_{x}$, $\alpha_{y}$ and $\alpha_{z}$ are the material's thermal expansion coefficients and $\Delta T$ is the change in temperature.

The formulation above for generalized plane strain problem shares many similarities with that of the conventional plane stress or plane strain problem. All the differences are associated with the introduction of an additional degree of freedom for the out-of-plane direction.

!bibtex bibliography
