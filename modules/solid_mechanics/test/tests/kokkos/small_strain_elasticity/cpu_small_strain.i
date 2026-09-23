# Isotropic linear elasticity on a small block, assembled through the non-Kokkos material and
# kernel chain. This input produces the reference solution that kokkos_small_strain.i must
# reproduce, so the two discretize the same problem: the same mesh, the same elastic constants and
# the same prescribed displacements. The displacements are prescribed on both ends and differ in
# all three components, so the solution exercises every component of the elasticity tensor.
#
# youngs_modulus and poissons_ratio are chosen to give Lame parameters of exactly one,
# lambda = E nu / ((1 + nu) (1 - 2 nu)) = 1 and G = E / (2 (1 + nu)) = 1, which keeps the Jacobian
# entries of order one. PetscJacobianTester compares them against an absolute tolerance, which
# would be meaningless at the stiffnesses of a real material.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 2
    nx = 4
    ny = 2
    nz = 2
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [stress_divergence_x]
    type = StressDivergenceTensors
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    component = 0
  []
  [stress_divergence_y]
    type = StressDivergenceTensors
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    component = 1
  []
  [stress_divergence_z]
    type = StressDivergenceTensors
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    component = 2
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.5
    poissons_ratio = 0.25
  []
  [strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[BCs]
  [fixed_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [fixed_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0
  []
  [fixed_z]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0
  []
  [pull_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.001
  []
  [pull_y]
    type = DirichletBC
    variable = disp_y
    boundary = right
    value = 0.002
  []
  [pull_z]
    type = DirichletBC
    variable = disp_z
    boundary = right
    value = -0.003
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  # A direct solve keeps the comparison between the two chains limited by assembly roundoff
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Postprocessors]
  # Norms over the whole domain, rather than boundary extrema, so that the comparison is sensitive
  # to the interior solution and not just to the prescribed values
  [disp_x_l2]
    type = ElementL2Norm
    variable = disp_x
  []
  [disp_y_l2]
    type = ElementL2Norm
    variable = disp_y
  []
  [disp_z_l2]
    type = ElementL2Norm
    variable = disp_z
  []
[]

[Outputs]
  csv = true
[]
