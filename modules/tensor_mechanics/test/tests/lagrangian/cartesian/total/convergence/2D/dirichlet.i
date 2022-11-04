# Simple 2D plane strain test

[GlobalParams]
  displacements = 'disp_x disp_y'
  large_kinematics = true
[]

[Variables]
  [disp_x]
  []
  [disp_y]
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
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
[]

[Functions]
  [pullx]
    type = ParsedFunction
    expression = '0.5 * t'
  []
  [pully]
    type = ParsedFunction
    expression = '-0.3 * t'
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
    type = FunctionDirichletBC
    boundary = right
    variable = disp_x
    function = pullx
    preset = true
  []
  [pull_y]
    type = FunctionDirichletBC
    boundary = top
    variable = disp_y
    function = pully
    preset = true
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
  dt = 0.2
  dtmin = 0.2
  end_time = 1.0
[]

[Postprocessors]
  [nonlin]
    type = NumNonlinearIterations
  []
[]

[Outputs]
  exodus = false
  csv = true
[]
