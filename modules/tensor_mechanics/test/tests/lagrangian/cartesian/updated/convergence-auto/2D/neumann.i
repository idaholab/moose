# Simple 2D plane strain test

[GlobalParams]
  displacements = 'disp_x disp_y'
  large_kinematics = true
  stabilize_strain = true
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[ICs]
  [disp_x]
    type = RandomIC
    variable = disp_x
    min = -0.01
    max = 0.01
  []
  [disp_y]
    type = RandomIC
    variable = disp_y
    min = -0.01
    max = 0.01
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[Kernels]
  [sdx]
    type = UpdatedLagrangianStressDivergence
    variable = disp_x
    component = 0
    use_displaced_mesh = true
  []
  [sdy]
    type = UpdatedLagrangianStressDivergence
    variable = disp_y
    component = 1
    use_displaced_mesh = true
  []
[]

[Functions]
  [pullx]
    type = ParsedFunction
    expression = '50000 * t'
  []
  [pully]
    type = ParsedFunction
    expression = '-30000 * t'
  []
[]

[BCs]
  [leftx]
    type = DirichletBC
    preset = true
    boundary = left
    variable = disp_x
    value = 0.0
  []
  [lefty]
    type = DirichletBC
    preset = true
    boundary = left
    variable = disp_y
    value = 0.0
  []
  [pull_x]
    type = FunctionNeumannBC
    boundary = right
    variable = disp_x
    function = pullx
  []
  [pull_y]
    type = FunctionNeumannBC
    boundary = top
    variable = disp_y
    function = pully
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
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12

  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

