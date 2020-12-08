# Compute Elasticity Tensor

!syntax description /Materials/ComputeElasticityTensor

## Description

The material `ComputeElasticityTensor` builds the elasticity (stiffness) tensor with various
user-selected material symmetry options.  `ComputeElasticityTensor` also rotates the elasticity
tensor during the initial time step only; this class does not rotate the elasticity tensor during the
simulation.  The initial rotation is performed if the user provides arguments to the three Euler
angle parameters; the Bunge Euler angles provided in this class are used to perform passive (from the
sample to the crystal) rotations using the [extrinsic $Z_1 X_2 Z_3$ convention](https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix)
to build the rotation matrix $R$. Alternatively, the 3x3 rotation matrix can be defined directly in the input file.
Rotations to the elasticity tensor are applied as
\begin{equation}
\label{eq:rotate_elasticity_tensor}
  C_{ijkl} = R_{im} R_{jn} R_{ko} R_{lp} C_{mnop}
\end{equation}
See [below](#rotation-examples) for examples of tensor rotations.

For a general stiffness tensor with 21 independent components, the elasticity tensor within the
tensor mechanics module can be represented with the notation shown in
[eq:rank4tensor_aux_indices].  Nonetheless, the full Rank-4 tensor with all 81 components is
created by `ComputeElasticityTensor`.
\begin{equation}
\label{eq:rank4tensor_aux_indices}
  \begin{aligned}
        C_{ijkl} \implies & \underbrace{\begin{bmatrix}
                      C_{11} & C_{12} & C_{13} & C_{14} & C_{15} & C_{16} \\
                      C_{21} & C_{22} & C_{23} & C_{24} & C_{25} & C_{26} \\
                      C_{31} & C_{32} & C_{33} & C_{34} & C_{35} & C_{36} \\
                      C_{41} & C_{42} & C_{43} & C_{44} & C_{45} & C_{46} \\
                      C_{51} & C_{52} & C_{53} & C_{54} & C_{55} & C_{56} \\
                      C_{61} & C_{62} & C_{63} & C_{64} & C_{65} & C_{66}
                      \end{bmatrix}}_{\text{textbook engineering notation}} \\[5.0em]
         \implies & \underbrace{\begin{bmatrix}
                                   C_{1111} & C_{1122} & C_{1133} & C_{1123} & C_{1131} & C_{1112} \\
                                   C_{2211} & C_{2222} & C_{2233} & C_{2223} & C_{2231} & C_{2212} \\
                                   C_{3311} & C_{3322} & C_{3333} & C_{3323} & C_{3331} & C_{3312} \\
                                   C_{2311} & C_{2322} & C_{2333} & C_{2323} & C_{2331} & C_{2312} \\
                                   C_{3111} & C_{3122} & C_{3133} & C_{3123} & C_{3131} & C_{3112} \\
                                   C_{1211} & C_{1222} & C_{1233} & C_{1223} & C_{1231} & C_{1212}
                                   \end{bmatrix}}_{\text{textbook Einstein index notation}} \\[5.0em]
         \implies & \underbrace{\begin{bmatrix}
                      C_{0000} & C_{0011} & C_{0022} & C_{0012} & C_{0020} & C_{0001} \\
                      C_{1100} & C_{1111} & C_{1122} & C_{1112} & C_{1120} & C_{1101} \\
                      C_{2200} & C_{2211} & C_{2222} & C_{2212} & C_{2220} & C_{2201} \\
                      C_{1200} & C_{1211} & C_{1222} & C_{1212} & C_{1220} & C_{1201} \\
                      C_{2000} & C_{2011} & C_{2022} & C_{2012} & C_{2020} & C_{2001} \\
                      C_{0100} & C_{0111} & C_{0122} & C_{0112} & C_{0120} & C_{0101}
                      \end{bmatrix}}_{\text{Compute Elasticity Tensor indices}}
  \end{aligned}
\end{equation}

There are several different material symmetry options that a user can apply to build the elasticity
tensor for a mechanics simulation that are discussed below.

## General Symmetry

The fill method `symmetric21` is used to create the elasticity tensor for a linear hyperelastic
material with 21 independent components: the symmetries shown in
[eq:symmetric21_cijkl_cases] are used to determine the independent components
[!citep](slaughter2012linearized).
\begin{equation}
\label{eq:symmetric21_cijkl_cases}
  \begin{aligned}
    C_{ijkl} & = C_{jikl} \impliedby \quad \text{satisfies angular momentum} \\
    C_{ijkl} & = C_{ijlk} \impliedby \quad \text{symmetric strain tensor assumption} \\
    C_{ijkl} & = C_{klij} \impliedby \quad \text{linear hyperelastic material assumption}
  \end{aligned}
\end{equation}

### Example Input File Syntax

!listing modules/combined/test/tests/linear_elasticity/tensor.i block=Materials/elasticity_tensor

which shows the expected order of the elasticity tensor components in the input argument string.

## Orthotropic Symmetry

The fill method `symmetric9` is appropriate for materials with three orthotropic planes of symmetry
[!citep](malvern1969introduction), and is often used for simulations of anistropic materials such as
cubic crystals.  The enginering elasticity tensor notation, [eq:rank4tensor_aux_indices],
for an orthotropic material is given in [eq:symmetric9_fill_method]
\begin{equation}
\label{eq:symmetric9_fill_method}
C_{ijkl}^{orthotropic} = \begin{bmatrix}
              C_{11} & C_{12} & C_{13} &      0 &      0 &      0 \\
              C_{12} & C_{22} & C_{23} &      0 &      0 &      0 \\
              C_{13} & C_{23} & C_{33} &      0 &      0 &      0 \\
                   0 &      0 &      0 & C_{44} &      0 &      0 \\
                   0 &      0 &      0 &      0 & C_{55} &      0 \\
                   0 &      0 &      0 &      0 &      0 & C_{66}
              \end{bmatrix}
\end{equation}

The user can also select the fill method `orthotropic`, which generates an elasticity tensor based directly on material parameters. That is, the elasticity tensor is computed via moduli of elasticity, Poisson's ratios, and shear stiffnesses, see [eq:orhotropic_fill_method]
\begin{equation}
\label{eq:orhotropic_fill_method}
C_{ijkl}^{orthotropic} = \begin{bmatrix}
              E_{1} (1 - \nu_{23} \nu_{32})/k & E_{1} (\nu_{23} \nu_{31} + \nu_{21})/k & E_{1} (\nu_{21} \nu_{32} + \nu_{31})/k  &      0 &      0 &      0 \\
              E_{2} (\nu_{13} \nu_{32} + \nu_{12})/k & E_{2} (1 - \nu_{13} \nu_{31})/k & E_{2} (\nu_{12} \nu_{31} + \nu_{32})/k &      0 &      0 &      0 \\
              E_{3} (\nu_{12} \nu_{23} + \nu_{13})/k & E_{3} (\nu_{13} \nu_{21} + \nu_{23})/k & E_{3} (1 - \nu_{12} \nu_{21})/k &      0 &      0 &      0 \\
                   0 &      0 &      0 & G_{12} &      0 &      0 \\
                   0 &      0 &      0 &      0 & G_{31} &      0 \\
                   0 &      0 &      0 &      0 &      0 & G_{23}
              \end{bmatrix}
\end{equation}
where $k = 1 - \nu_{12} \nu_{21} - \nu_{23} \nu_{32} - \nu_{31} \nu_{13} - \nu_{12} \nu_{23} \nu_{31} - \nu_{21} \nu_{32} \nu_{13}$.


### Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Materials/elasticity_tensor

In the Einstein index notation shown in [eq:rank4tensor_aux_indices], the parameter `C_ijkl`
expects the elasticity components in the order `C_ijkl = '1111 1122 1133 2222 2233 3333 2323 3131
1212'` for the `symmetric9` fill method option. Note that, in this case, the method `symmetric9` is used
to enter an isotropic elasticity tensor.

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic_anisotropy/3d_bar_orthotropic_full_rotation.i block=Materials/elasticity_tensor

For the `orthotropic` fill method, the material parameters need to be referred to the global frame and introduced in the following order: $E_{11}\;E_{22}\;E_{33}\;G_{12}\;G_{23}\;G_{31}\;\nu_{21}\;\nu_{31}\;\nu_{32}\;\nu_{12}\;\nu_{13}\;\nu_{23}$. For cases where axes of orthotropy do not coincide with global axes, Euler angles can be provided to perform a general rotation of the elasticity tensor at the beginning of the simulation.


## Linear Isotropic Symmetry

The two constant isotropic symmetry fill methods `symmetric_isotropic` and `symmetric_isotropic_E_nu`
are used in the dedicated isotropic elasticity tensor
[ComputeIsotropicElasticityTensor](/ComputeIsotropicElasticityTensor.md).  These two fill methods use
the symmetries shown in [eq:symmetric_isotropic_fill_method] to build the elasticity tensor.
\begin{equation}
\label{eq:symmetric_isotropic_fill_method}
C_{ijkl} = C_{klij} = C_{jikl} = C_{jilk}
\end{equation}
Please see the documentation page for [ComputeIsotropicElasticityTensor](/ComputeIsotropicElasticityTensor.md) for details and examples of the input file syntax for linear elastic isotropic elasticity tensors.

## Antisymmetric Isotropic Symmetry

The fill method `antisymmetric_isotropic` is used for an antisymmetric isotropic material in a shear
case.  The elasticity tensor is built using the symmetries shown in
[eq:antisymmetric_isotropic_fill_method]
\begin{equation}
\label{eq:antisymmetric_isotropic_fill_method}
C_{ijkl}^{antisymmetric-isotropic} = \kappa e_{ijm} e_{klm}
\end{equation}
where $e$ is the permutation tensor and $m$ is the summation index.

## Transverse Isotropic (Axisymmetric)

The fill method `axisymmetric_rz` is used for materials which are isotropic with respect to an axis of symmetry, such as a material composed of fibers which are parallel to the axis of symmetry [!citep](slaughter2012linearized).
The engineering notation matrix in this case is shown by [eq:axisymmetric_rz_fill_method].
\begin{equation}
\label{eq:axisymmetric_rz_fill_method}
C_{ijkl}^{axisymmetric} = \begin{bmatrix}
              C_{11} & C_{12} & C_{13} &      0 &      0 &      0 \\
              C_{12} & C_{11} & C_{13} &      0 &      0 &      0 \\
              C_{13} & C_{13} & C_{33} &      0 &      0 &      0 \\
                   0 &      0 &      0 & C_{44} &      0 &      0 \\
                   0 &      0 &      0 &      0 & C_{44} &      0 \\
                   0 &      0 &      0 &      0 &      0 & \frac{1}{2} \left( C_{11} - C_{12} \right)
              \end{bmatrix}
\end{equation}

### Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/isotropic_elasticity_tensor/2D-axisymmetric_rz_test.i
         block=Materials/elasticity_tensor

In the Einstein index notation shown in [eq:rank4tensor_aux_indices], the parameter `C_ijkl`
expects the elasticity components in the order `C_ijkl = '1111, 1122, 1133, 3333, 2323'` for the
`axisymmetric_rz` fill method option.

## Principal Directions for Stress and Strain

The fill method `principal` is appropriate for the case when the principal directions of strain and
stress align.  The engineering notation representation of the elasticity tensor is shown in
[eq:principal_fill_method].
\begin{equation}
\label{eq:principal_fill_method}
C_{ijkl}^{orthotropic} = \begin{bmatrix}
              C_{11} & C_{12} & C_{13} &      0 &      0 &      0 \\
              C_{21} & C_{22} & C_{23} &      0 &      0 &      0 \\
              C_{31} & C_{32} & C_{33} &      0 &      0 &      0 \\
                   0 &      0 &      0 &      0 &      0 &      0 \\
                   0 &      0 &      0 &      0 &      0 &      0 \\
                   0 &      0 &      0 &      0 &      0 &      0
              \end{bmatrix}
\end{equation}

In the Einstein index notation shown in [eq:rank4tensor_aux_indices], the parameter `C_ijkl`
expects the elasticity components in the order `C_ijkl = '1111 1122 1133 2211 2222 2233 3311 3322
3333'` for the `principal` fill method option.

## Cosserat Elasticity Specific Fill Methods

The following fill methods are available within `ComputeElasticityTensor`, but the use cases for
these methods fall within the Cosserat applications which do not preserve the equilibruim of angular
momentum.

### General Isotropic Symmetry

The fill method `general_isotropic` is used for the case of three independent components of an
elasticity tensor, [eq:general_isotropic_cijkl].
\begin{equation}
\label{eq:general_isotropic_cijkl}
C_{ijkl}^{isotropic} = \lambda \delta_{ij} \delta_{kl} + \mu \delta_{ik} \delta_{ji} + \kappa \delta_{il} \delta_{jk}
\end{equation}

This fill method case is used in the child class
[ComputeCosseratElasticityTensor](/ComputeCosseratElasticityTensor.md); please see the documentation
for [ComputeCosseratElasticityTensor](/ComputeCosseratElasticityTensor.md) for details and examples
of the input file syntax.

### General Antisymmetric

The fill method `antisymmetric` builds an antisymmetric elasticity tensor for a shear-only case.  The
symmetries shown in [eq:antisymmetric_symmetries] are used to create the complete tensor
\begin{equation}
\label{eq:antisymmetric_symmetries}
C_{ijkl} = - C_{jikl} = - C_{ijlk} = C_{klij}
\end{equation}
and the engineering notation representation of the anitsymmetric elasticity tensor is given in
[eq:antisymmetric_fill_method].
\begin{equation}
\label{eq:antisymmetric_fill_method}
C_{ijkl}^{antisymmetric} = \begin{bmatrix}
                   0 &      0 &      0 &      0 &      0 &      0 \\
                   0 &      0 &      0 &      0 &      0 &      0 \\
                   0 &      0 &      0 &      0 &      0 &      0 \\
                   0 &      0 &      0 &  C_{44} & -C_{54} &  C_{64} \\
                   0 &      0 &      0 & -C_{54} & -C_{55} & -C_{65} \\
                   0 &      0 &      0 &  C_{64} & -C_{65} &  C_{66}
              \end{bmatrix}
\end{equation}

This fill method case is used in the child class
[ComputeCosseratElasticityTensor](/ComputeCosseratElasticityTensor.md); please see the documentation
for [ComputeCosseratElasticityTensor](/ComputeCosseratElasticityTensor.md) for details and examples
of the input file syntax.

### No Symmetry

The `general` fill method for the Compute Elasticity Tensor class does not make any assumptions about
symmetry for the elasticity tensor and requires all 81 components of the stiffness tensor as an input
string.  This fill method case is used in the child class
[ComputeCosseratElasticityTensor](/ComputeCosseratElasticityTensor.md); please see the documentation
for [ComputeCosseratElasticityTensor](/ComputeCosseratElasticityTensor.md) for details and examples
of the input file syntax.

## Rotation Examples

Since the elasticity tensor is defined with respect to a given crystal orientation which may be
different than the simulation coordinate frame, one may wish to apply a rotation to the
elasticity tensor. Some example use cases include simulating a specific crystal plane
in a 2D simulation or embedding a secondary phase in a matrix phase. The rotation ensures the
correct stress is generated when a given strain is applied. Strain is calculated in the "sample"
or "simulation" reference frame and so the proper "sample to crystal" rotation must be applied.
As shown in [eq:rotate_elasticity_tensor], a rotation matrix is needed for this operation, which
can be built with Euler angles or entered in the input file directly.

### Rotation About An Axis

Suppose we wish to rotate the elasticity tensor about the z-axis by 30 degrees. The rotation matrix
for rotating a vector by 30 degrees about the z-axis (an "active" rotation matrix) is
\begin{equation}
\label{eq:z_axis_rotation_30_degrees}
R_z = \begin{bmatrix}
          \frac{\sqrt{3}}{2} & -\frac{1}{2}       &  0  \\
          \frac{1}{2}        & \frac{\sqrt{3}}{2} &  0  \\
          0                  &      0             &  1
      \end{bmatrix}
  \approx
      \begin{bmatrix}
             0.8660254 & -0.5      &  0  \\
             0.5       & 0.8660254 &  0  \\
             0         & 0         &  1
      \end{bmatrix}
\end{equation}
Such a rotation can be implemented by directly supplying the rotation matrix in the input file
using the `rotation_matrix` parameter in `ComputeElasticityTensor`.

### Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/elasticitytensor/rotation_matrix_1_rotation.i
         block=Materials/elasticity_matrix

Note that the same rotation can be applied using Euler angles. MOOSE expects the "passive"
(Bunge) convention, so the input angle is -30 degrees. Also note that since the $Z_1 X_2 Z_3$ convention
is used, and only a single rotation is needed, the angle could be entered as either
`euler_angle_1` or `euler_angle_3`.

### Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/elasticitytensor/rotation_matrix_1_rotation.i
         block=Materials/elasticity_euler

### Orientation Relationship Rotation

Rather than a specific axis-angle rotation, sometimes the elasticity tensor must be rotated to
satisfy a particular orientation relationship. In this case, the basis vectors of the rotated
coordinate system are known, and the corresponding rotation matrix can be built using the
unit basis vectors as rows of the rotation matrix. For example suppose we have the following
rotation matrix that can be used in the case where the $\left<111\right>$ direction of our
rotated or "crystal" system points along the z-axis of our simulation or "sample" system.
\begin{equation}
\label{eq:orientation_rotation}
R = \begin{bmatrix}
        \frac{\sqrt{2}}{2} & \frac{\sqrt{6}}{6} & \frac{\sqrt{3}}{3}  \\
       -\frac{\sqrt{2}}{2} & \frac{\sqrt{6}}{6} & \frac{\sqrt{3}}{3}  \\
        0                  & -\frac{\sqrt{6}}{3} & \frac{\sqrt{3}}{3}
    \end{bmatrix}
 \approx
    \begin{bmatrix}
         0.70710678 &  0.40824829 & 0.57735027  \\
        -0.70710678 &  0.40824829 & 0.57735027  \\
         0          & -0.81649658 & 0.57735027
    \end{bmatrix}
\end{equation}
To verify, we apply this rotation matrix to the $\hat{z}=\left<001\right>$ direction of our "sample" or
simulation frame: $R\hat{z}=\left<111\right>$, meaning it correctly converts directions from "sample" to
"crystal" frame, which is a "passive" rotation.

### Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/elasticitytensor/rotation_matrix_2_rotations.i
         block=Materials/elasticity_matrix

The rotation matrix in [eq:orientation_rotation] can also be built in an "active" sense
by doing a 45 degree rotation about the z-axis and then a ~54.7 degree ($\arccos\left(1/\sqrt{3}\right)$ radians)
rotation about the x-axis, then taking the transpose. Therefore these
are the corresponding Euler angles to be used since we rotated the elasticity tensor by the "passive" matrix
rather than the "active" matrix in the previous example. However, where more than 1 rotation is needed, the
order of rotations matter. Since the Euler angle convention in MOOSE uses "extrinsic" rotations, the
order must be reversed.

### Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/elasticitytensor/rotation_matrix_2_rotations.i
         block=Materials/elasticity_euler

!syntax parameters /Materials/ComputeElasticityTensor

!syntax inputs /Materials/ComputeElasticityTensor

!syntax children /Materials/ComputeElasticityTensor

!bibtex bibliography
