
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
    type = ADTimeDerivative
    variable = temp
  [../]
  [./heat_conduction]
    type = Diffusion
    variable = temp
  [../]
  [./heat]
    type = ADBodyForce
    variable = temp
    value = 0
  [../]
[]

[BCs]
  [./right]
    type = ADConvectiveHeatFluxBC
    variable = temp
    boundary = 'right'
    T_infinity = T_inf
    heat_transfer_coefficient = htc
  [../]
[]

[Materials]
  [chf_mat]
    type = ADConvectiveHeatFluxTest
    temperature = temp
    boundary = 'right'
  []
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
    type = SideDiffusiveFluxAverage
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
