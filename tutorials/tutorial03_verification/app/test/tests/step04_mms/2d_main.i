[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    ymax = 0
    ymin = -0.2
    nx = 20
    ny = 4
  []
[]

[Variables]
  [T]
  []
[]

[ICs]
  [T_O]
    type = ConstantIC
    variable = T
    value = 263.15
  []
[]

[Functions]
  [source]
    type = ParsedFunction
    symbol_names = 'hours shortwave kappa'
    symbol_values = '9     650      40'
    expression = 'shortwave*sin(0.5*x*pi)*exp(kappa*y)*sin(1/(hours*3600)*pi*t)'
  []
[]

[Kernels]
  [T_time]
    type = HeatConductionTimeDerivative
    variable = T
    density_name = 150
    specific_heat = 2000
  []
  [T_cond]
    type = HeatConduction
    variable = T
    diffusion_coefficient = 0.01
  []
  [T_source]
    type = HeatSource
    variable = T
    function = source
  []
[]

[BCs]
  [top]
    type = NeumannBC
    boundary = top
    variable = T
    value = -5
  []
  [bottom]
    type = DirichletBC
    boundary = bottom
    variable = T
    value = 263.15
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  dt = 600 # 10 min
  end_time = 32400 # 9 hour
[]

[Outputs]
  exodus = true
[]
