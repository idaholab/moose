[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    xmax = 0.03
    nx = 200
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
    value = 300
  []
[]

[Kernels]
  [T_time]
    type = HeatConductionTimeDerivative
    variable = T
    density_name = 7800
    specific_heat = 450
  []
  [T_cond]
    type = HeatConduction
    variable = T
    diffusion_coefficient = 80.2
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = T
    boundary = left
    value = 7e5
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  dt = 0.01
  end_time = 1
[]

[Outputs]
  exodus = true
  csv = true
[]

[Functions]
  [T_exact]
    type = ParsedFunction
    symbol_names = 'k    rho  cp  T0  qs'
    symbol_values = '80.2 7800 450 300 7e5'
    expression = 'T0 + '
            'qs/k*(2*sqrt(k/(rho*cp)*t/pi)*exp(-x^2/(4*k/(rho*cp)*(t+1e-50))) - '
            'x*(1-erf(x/(2*sqrt(k/(rho*cp)*(t+1e-50))))))'
  []
[]

[Postprocessors]
  [error]
    type = NodalL2Error
    variable = T
    function = T_exact
  []
  [h]
    type = AverageElementSize
  []
[]


[VectorPostprocessors]
  [T_exact]
    type = LineFunctionSampler
    functions = T_exact
    start_point = '0 0 0'
    end_point = '0.03 0 0'
    num_points = 200
    sort_by = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_simulation]
    type = LineValueSampler
    variable = T
    start_point = '0 0 0'
    end_point = '0.03 0 0'
    num_points = 200
    sort_by = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
