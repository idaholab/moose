# Correctness check for the generalized midpoint rule (alpha != 1).
#
# Single 3D element with a linearly-ramped Dirichlet BC on disp_x (u_x = ramp_rate * t * x).
# The other faces are constrained so the only displacement is a uniform stretch in x.
#
# Expected analytical values at each step (alpha-weighted vs actual F_{11}):
#   F^alpha_{11}(t_n)  = 1 + alpha * (ramp_rate * t_n) + (1 - alpha) * (ramp_rate * t_{n-1})
#   F^actual_{11}(t_n) = 1 + ramp_rate * t_n
#
# With ramp_rate = 0.01, dt = 1, three steps (t = 1, 2, 3):
#   alpha = 0.5: F^alpha = 1.005, 1.015, 1.025; F^actual = 1.010, 1.020, 1.030
#   alpha = 1.0: F^alpha = F^actual = 1.010, 1.020, 1.030
#
# Drive the cli_args from the tests file to select alpha.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
[]

[Mesh]
  [single]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
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

[Functions]
  [ramp]
    type = ParsedFunction
    expression = '0.01 * t * x'
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [right_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = ramp
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [back_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
    poissons_ratio = 0.3
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [compute_strain]
    type = ComputeLagrangianStrain
  []
  [F_alpha_11]
    type = RankTwoCartesianComponent
    rank_two_tensor = deformation_gradient
    property_name = F_alpha_11
    index_i = 0
    index_j = 0
  []
  [F_actual_11]
    type = RankTwoCartesianComponent
    rank_two_tensor = actual_deformation_gradient
    property_name = F_actual_11
    index_i = 0
    index_j = 0
  []
[]

[Postprocessors]
  [F_alpha_11]
    type = ElementAverageMaterialProperty
    mat_prop = F_alpha_11
  []
  [F_actual_11]
    type = ElementAverageMaterialProperty
    mat_prop = F_actual_11
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1
  end_time = 3
[]

[Outputs]
  csv = true
[]
