[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 30
  ny = 30
  nz = 30
[]

[Variables]
  [temperature]
    initial_condition = 500.0
  []
[]

[AuxVariables]
  [source]
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temperature
  []
  [source]
    type = CoupledForce
    variable = temperature
    v = source
  []
[]

[AuxKernels]
  [source]
    type = ParsedAux
    variable = source
    function = 'temperature*7*t'
    coupled_variables = 'temperature'
    execute_on = 'linear'
    use_xyzt = true
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = 'left'
    value = 500.0
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = 'right'
    value = 600.0
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = 'top'
    value = 650.0
  []
  [front]
    type = DirichletBC
    variable = temperature
    boundary = 'front'
    value = 650.0
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1.5'
  []
[]

[Postprocessors]
  [timestep_ctr]
    type = NumTimeSteps
  []
[]


[Executioner]
  type = Transient
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-4
  dt = 1
  num_steps = 10
[]

[Outputs]
  exodus = false
  csv = true
  hide = 'source temperature'
[]

