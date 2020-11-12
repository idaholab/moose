
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
    value = 0
    point = '0.3 0.8 0'
  [../]
  [./ar01]
    type = ConstantPointSource
    variable = temperature
    value = 0
    point = '0.3 0.6 0'
  [../]
  [./ar02]
    type = ConstantPointSource
    variable = temperature
    value = 0
    point = '0.3 0.4 0'
  [../]
  [./ar03]
    type = ConstantPointSource
    variable = temperature
    value = 0
    point = '0.3 0.2 0'
  [../]
  [./ar04]
    type = ConstantPointSource
    variable = temperature
    value = 0
    point = '0.7 0.8 0'
  [../]
  [./ar05]
    type = ConstantPointSource
    variable = temperature
    value = 0
    point = '0.7 0.6 0'
  [../]
  [./ar06]
    type = ConstantPointSource
    variable = temperature
    value = 0
    point = '0.7 0.4 0'
  [../]
  [./ar07]
    type = ConstantPointSource
    variable = temperature
    value = 0
    point = '0.7 0.2 0'
  [../]
[]


[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 0
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
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

[Postprocessors]
[dr00]
  type = PointValue
  variable = temperature
  point = '0.5 0.1 0'
[]
[dr01]
  type = PointValue
  variable = temperature
  point = '0.5 0.2 0'
[]
[dr02]
  type = PointValue
  variable = temperature
  point = '0.5 0.3 0'
[]
[dr03]
  type = PointValue
  variable = temperature
  point = '0.5 0.4 0'
[]
[dr04]
  type = PointValue
  variable = temperature
  point = '0.5 0.5 0'
[]
[dr05]
  type = PointValue
  variable = temperature
  point = '0.5 0.6 0'
[]
[dr06]
  type = PointValue
  variable = temperature
  point = '0.5 0.7 0'
[]
[dr07]
  type = PointValue
  variable = temperature
  point = '0.5 0.8 0'
[]
[dr08]
  type = PointValue
  variable = temperature
  point = '0.5 0.9 0'
[]
[dr09]
  type = PointValue
  variable = temperature
  point = '0.9 0.1 0'
[]
[dr10]
  type = PointValue
  variable = temperature
  point = '0.9 0.2 0'
[]
[dr11]
  type = PointValue
  variable = temperature
  point = '0.9 0.3 0'
[]
[dr12]
  type = PointValue
  variable = temperature
  point = '0.9 0.4 0'
[]
[dr13]
  type = PointValue
  variable = temperature
  point = '0.9 0.5 0'
[]
[dr14]
  type = PointValue
  variable = temperature
  point = '0.9 0.6 0'
[]
[dr15]
  type = PointValue
  variable = temperature
  point = '0.9 0.7 0'
[]
[dr16]
  type = PointValue
  variable = temperature
  point = '0.9 0.8 0'
[]
[dr17]
  type = PointValue
  variable = temperature
  point = '0.9 0.9 0'
[]
[dr18]
  type = PointValue
  variable = temperature
  point = '0.1 0.1 0'
[]
[dr19]
  type = PointValue
  variable = temperature
  point = '0.1 0.2 0'
[]
[dr20]
  type = PointValue
  variable = temperature
  point = '0.1 0.3 0'
[]
[dr21]
  type = PointValue
  variable = temperature
  point = '0.1 0.4 0'
[]
[dr22]
  type = PointValue
  variable = temperature
  point = '0.1 0.5 0'
[]
[dr23]
  type = PointValue
  variable = temperature
  point = '0.1 0.6 0'
[]
[dr24]
  type = PointValue
  variable = temperature
  point = '0.1 0.7 0'
[]
[dr25]
  type = PointValue
  variable = temperature
  point = '0.1 0.8 0'
[]
[dr26]
  type = PointValue
  variable = temperature
  point = '0.1 0.9 0'
[]
[]

# should be able to do all this in the transfer  line 40 of sampler Receiver
[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]


[Outputs]
  console = false
  exodus = true
  file_base = 'forward'
[]
