
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
[a00]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.5 0.1 0'
[]
[a01]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.5 0.2 0'
[]
[a02]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.5 0.3 0'
[]
[a03]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.5 0.4 0'
[]
[a04]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.5 0.5 0'
[]
[a05]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.5 0.6 0'
[]
[a06]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.5 0.7 0'
[]
[a07]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.5 0.8 0'
[]
[a08]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.5 0.9 0'
[]
[a09]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.9 0.1 0'
[]
[a10]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.9 0.2 0'
[]
[a11]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.9 0.3 0'
[]
[a12]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.9 0.4 0'
[]
[a13]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.9 0.5 0'
[]
[a14]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.9 0.6 0'
[]
[a15]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.9 0.7 0'
[]
[a16]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.9 0.8 0'
[]
[a17]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.9 0.9 0'
[]
[a18]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.1 0.1 0'
[]
[a19]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.1 0.2 0'
[]
[a20]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.1 0.3 0'
[]
[a21]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.1 0.4 0'
[]
[a22]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.1 0.5 0'
[]
[a23]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.1 0.6 0'
[]
[a24]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.1 0.7 0'
[]
[a25]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.1 0.8 0'
[]
[a26]
  type = ConstantPointSource
  variable = temperature
  value = 7.5
  point = '0.1 0.9 0'
[]
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
  [ar00]
    type = PointValue
    variable = temperature
    point = '0.3 0.8 0'
  []
  [ar01]
    type = PointValue
    variable = temperature
    point = '0.3 0.6 0'
  []
  [ar02]
    type = PointValue
    variable = temperature
    point = '0.3 0.4 0'
  []
  [ar03]
    type = PointValue
    variable = temperature
    point = '0.3 0.2 0'
  []
  [ar04]
    type = PointValue
    variable = temperature
    point = '0.7 0.8 0'
  []
  [ar05]
    type = PointValue
    variable = temperature
    point = '0.7 0.6 0'
  []
  [ar06]
    type = PointValue
    variable = temperature
    point = '0.7 0.4 0'
  []
  [ar07]
    type = PointValue
    variable = temperature
    point = '0.7 0.2 0'
  []
[]

# should be able to do all this in the transfer  line 40 of sampler Receiver
[Controls]
  [adjointReceiver]
    type = ControlsReceiver
  []
[]


[Outputs]
  console = false
  exodus = true
  file_base = 'adjoint'
[]
