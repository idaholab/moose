
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 20
  xmax = 1
  ymax = 2
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
  [./pt0]
    type = ConstantPointSource
    variable = temperature
    value = 10
    point = '0.3 0.3'
  [../]
  [./pt1]
    type = ConstantPointSource
    variable = temperature
    value = 10
    point = '0.4 1.0'
  [../]
  [./pt2]
    type = ConstantPointSource
    variable = temperature
    value = 10
    point = '0.8 0.5'
  [../]
  [./pt3]
    type = ConstantPointSource
    variable = temperature
    value = 10
    point = '0.8 0.6'
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
  [adjoint_pt_0]
    type = PointValue
    variable = temperature
    point = '0.2 0.2 0'
  []
  [adjoint_pt_1]
    type = PointValue
    variable = temperature
    point = '0.2 0.8 0'
  []
  [adjoint_pt_2]
    type = PointValue
    variable = temperature
    point = '0.8 0.2 0'
  []
[]

# should be able to do all this in the transfer  line 40 of sampler Receiver
[Controls]
  [adjointReceiver]
    type = ControlsReceiver
  []
[]


[Outputs]
  console = true
  exodus = true
  file_base = 'adjoint'
[]
