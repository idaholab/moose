
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100
    xmax = 1
    ymax = 1
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
[]

[DiracKernels]
  [./ar00]
    type = ConstantPointSource
    variable = temperature
    value = 10
    point = '0.3 0.8 0'
  [../]
  [./ar01]
    type = ConstantPointSource
    variable = temperature
    value = 20
    point = '0.3 0.6 0'
  [../]
  [./ar02]
    type = ConstantPointSource
    variable = temperature
    value = 30
    point = '0.3 0.4 0'
  [../]
  [./ar03]
    type = ConstantPointSource
    variable = temperature
    value = 40
    point = '0.3 0.2 0'
  [../]
  [./ar04]
    type = ConstantPointSource
    variable = temperature
    value = 5
    point = '0.7 0.8 0'
  [../]
  [./ar05]
    type = ConstantPointSource
    variable = temperature
    value = 10
    point = '0.7 0.6 0'
  [../]
  [./ar06]
    type = ConstantPointSource
    variable = temperature
    value = 15
    point = '0.7 0.4 0'
  [../]
  [./ar07]
    type = ConstantPointSource
    variable = temperature
    value = 20
    point = '0.7 0.2 0'
  [../]
[]

[BCs]
  [all_sides]
    type = DirichletBC
    variable = temperature
    boundary = 'top bottom left right'
    value = 0
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 1
  []
[]

[Problem]#do we need this
  type = FEProblem
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [ln1]
    type = LineValueSampler
    start_point = '0.1 0.1 0'
    end_point =   '0.1 0.9 0'
    num_points = 9
    sort_by = id
    variable = temperature
  []
  [ln2]
    type = LineValueSampler
    start_point = '0.2 0.1 0'
    end_point =   '0.2 0.9 0'
    num_points = 9
    sort_by = id
    variable = temperature
  []
  [ln3]
    type = LineValueSampler
    start_point = '0.3 0.1 0'
    end_point =   '0.3 0.9 0'
    num_points = 9
    sort_by = id
    variable = temperature
  []
  [ln4]
    type = LineValueSampler
    start_point = '0.4 0.1 0'
    end_point =   '0.4 0.9 0'
    num_points = 9
    sort_by = id
    variable = temperature
  []
  [ln5]
    type = LineValueSampler
    start_point = '0.5 0.1 0'
    end_point =   '0.5 0.9 0'
    num_points = 9
    sort_by = id
    variable = temperature
  []
  [ln6]
    type = LineValueSampler
    start_point = '0.6 0.1 0'
    end_point =   '0.6 0.9 0'
    num_points = 9
    sort_by = id
    variable = temperature
  []
  [ln7]
    type = LineValueSampler
    start_point = '0.7 0.1 0'
    end_point =   '0.7 0.9 0'
    num_points = 9
    sort_by = id
    variable = temperature
  []
  [ln8]
    type = LineValueSampler
    start_point = '0.8 0.1 0'
    end_point =   '0.8 0.9 0'
    num_points = 9
    sort_by = id
    variable = temperature
  []
  [ln9]
    type = LineValueSampler
    start_point = '0.9 0.1 0'
    end_point =   '0.9 0.9 0'
    num_points = 9
    sort_by = id
    variable = temperature
  []
[]

[Postprocessors]
  [data_pt_0]
    type = PointValue
    variable = temperature
    point = '0.3 0.3 0'
  []
  [data_pt_1]
    type = PointValue
    variable = temperature
    point = '0.4 1.0 0'
  []
  [data_pt_2]
    type = PointValue
    variable = temperature
    point = '0.8 0.5 0'
  []
  [data_pt_3]
    type = PointValue
    variable = temperature
    point = '0.8 0.6 0'
  []
[]

[Outputs]
  console = true
  exodus = true
  csv = true
[]
