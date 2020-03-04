
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
[]

[Variables]
  [./temp]
    initial_condition = 200.0
  [../]
[]

[Kernels]
  [./heat_dt]
    type = TimeDerivative
    variable = temp
  [../]
  [./heat_conduction]
    type = Diffusion
    variable = temp
  [../]
  [./heat]
    type = BodyForce
    variable = temp
    value = 0
  [../]
[]

[BCs]
  [./right]
    type = ConvectiveHeatFluxBC
    variable = temp
    boundary = 'right'
    T_infinity = T_inf
    heat_transfer_coefficient = htc
    heat_transfer_coefficient_dT = dhtc_dT
  [../]
[]

[Materials]
  [./T_inf]
    type = ParsedMaterial
    f_name = T_inf
    args = temp
    function = 'temp + 1'
  [../]
  [./htc]
    type = ParsedMaterial
    f_name = htc
    args = temp
    function = 'temp / 100 + 1'
  [../]
  [./dhtc_dT]
    type = ParsedMaterial
    f_name = dhtc_dT
    args = temp
    function = '1 / 100'
  [../]
[]

[Postprocessors]
  [./left_temp]
    type = SideAverageValue
    variable = temp
    boundary = left
    execute_on = 'TIMESTEP_END initial'
  [../]
  [./right_temp]
    type = SideAverageValue
    variable = temp
    boundary = right
  [../]
  [./right_flux]
    type = SideFluxAverage
    variable = temp
    boundary = right
    diffusivity = 1
  [../]
[]

[Executioner]
  type = Transient

  num_steps = 10
  dt = 1

  nl_abs_tol = 1e-12
[]

[Outputs]
  [./out]
    type = CSV
    interval = 10
  [../]
[]
