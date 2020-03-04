
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 1
  xmin = 0.0
  xmax = 0.1
  ymin = 0.0
  ymax = 0.01
  elem_type = QUAD4
[]

[Variables]
  [./temp]
    initial_condition = 0.0
  [../]
[]

[BCs]
  [./FixedTempLeft]
    type = DirichletBC
    variable = temp
    boundary = left
    value = 0.0
  [../]
  [./FunctionTempRight]
    type = FunctionDirichletBC
    variable = temp
    boundary = right
    function = '100.0 * sin(pi*t/40)'
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
  [./HeatTdot]
    type = HeatConductionTimeDerivative
    variable = temp
  [../]
[]

[Materials]
  [./density]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '35.0 440.5 7200.0'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  l_tol = 1e-5
  nl_max_its = 50
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12

  dt = 1
  end_time = 32.0
[]

[Postprocessors]
  [./target_temp]
    type = NodalVariableValue
    variable = temp
    nodeid = 9
  [../]
[]

[Outputs]
  csv = true
[]
