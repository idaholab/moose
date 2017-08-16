[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 12
  xmax = 80
[]

[Variables]
  [./T]
  [../]
[]

[ICs]
  [./T_IC]
    type = FunctionIC
    variable = T
    function = '100*sin(pi*x/80)'
  [../]
[]

[Kernels]
  [./HeatDiff]
    type = HeatConduction
    variable = T
  [../]
  [./HeatTdot]
    type = ConsistentHeatCapacityTimeDerivative
    variable = T
  [../]
  [./source]
    type = BodyForce
    function = source
    variable = T
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = T
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = T
    boundary = right
    value = 0
  [../]
[]

[Functions]
  [./solution]
    type = ParsedFunction
    value = '100 * sin(pi * x / 80) * exp(-t/100)'
  [../]
  [./source]
    type = ParsedFunction
    value = '
             0.146502 * exp(-t/100) * sin(pi*x/80)
             -0.46 * exp(-t/50) * sin(pi*x/80) * sin(pi*x/80) / sqrt(exp(-t/100) * sin(pi*x/80))
             -0.092 * exp(-t/100) * sin(pi*x/80) * (1 + 10 * sqrt(exp(-t/100) * sin(pi*x/80)))
            '
  [../]
[]

[Materials]
  [./k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '0.95'
    block = 0
  [../]
  [./cp_rho]
    type = DerivativeParsedMaterial
    block = 0
    function = '0.092 * (1 + sqrt(T))'
    f_name = heat_capacity
    args = T
    derivative_order = 1
  [../]
[]

[Postprocessors]
  [./error]
    type = ElementL2Error
    variable = T
    function = solution
  [../]
[]

[Executioner]
  type = Transient
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-8
  l_tol = 1e-3
  dt = 1
  end_time = 4
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  print_perf_log = true
[]
