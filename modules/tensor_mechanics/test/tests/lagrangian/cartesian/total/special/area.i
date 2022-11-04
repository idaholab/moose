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
    nx = 1
    ny = 1
    nz = 1
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

[AuxVariables]
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [zstress]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 500'
  []
  [constant]
    type = ConstantFunction
    value = 1.0
  []
  [ratio]
    type = ParsedFunction
    symbol_names = 'sd su'
    symbol_values = 's_def s_undef'
    expression = 'sd / su'
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
  [boty]
    type = DirichletBC
    preset = true
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
  [backz]
    type = DirichletBC
    preset = true
    boundary = back
    variable = disp_z
    value = 0.0
  []
  [pull_z]
    type = FunctionNeumannBC
    boundary = front
    variable = disp_z
    function = zstress
  []
[]

[AuxKernels]
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000.0
    poissons_ratio = 0.25
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

[Postprocessors]
  [s_undef]
    type = SideIntegralVariablePostprocessor
    variable = stress_zz
    boundary = front
  []
  [s_def]
    type = SideIntegralVariablePostprocessor
    variable = stress_zz
    boundary = front
    use_displaced_mesh = true
  []
  [area_calc]
    type = FunctionValuePostprocessor
    function = ratio
  []
  [area]
    type = AreaPostprocessor
    boundary = front
    use_displaced_mesh = true
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
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Outputs]
  exodus = false
  csv = true
[]
