# Simple 3D test

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
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

[Functions]
  [pullx]
    type = ParsedFunction
    expression = '4000 * t'
  []
  [pully]
    type = ParsedFunction
    expression = '-2000 * t'
  []
  [pullz]
    type = ParsedFunction
    expression = '3000 * t'
  []
  [lambda_function]
    type = ParsedFunction
    expression = '1000.0*(t+1.0)'
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
  [leftz]
    type = DirichletBC
    preset = true
    boundary = left
    variable = disp_z
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
  [pull_z]
    type = FunctionNeumannBC
    boundary = right
    variable = disp_z
    function = pullz
  []
[]

[Materials]
  [compute_stress]
    type = ComputeNeoHookeanStress
    lambda = lambda
    mu = 67000.0
  []
  [lambda]
    type = GenericFunctionMaterial
    prop_names = lambda
    prop_values = lambda_function
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

  petsc_options = ''
  petsc_options_iname = '-pc_type -ksp_type'
  petsc_options_value = 'lu gmres'
  l_tol = 1e-8
  l_max_its = 100

  reuse_preconditioner = false
  reuse_preconditioner_max_linear_its = 20

  nl_max_its = 10
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 10.0
[]

[Reporters/iteration_info]
  type = IterationInfo
[]

[Outputs]
  exodus = false
  [./csv]
    type = CSV
    file_base = base_case
  [../]
[]
