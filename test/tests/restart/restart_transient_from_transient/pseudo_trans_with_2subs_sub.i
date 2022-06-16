[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  xmax = 0.3
  ymax = 0.3
[]

[AuxVariables]
  [power_density]
  []
[]

[Variables]
  [temp]
  []
[]

[Kernels]
  [heat_timedt]
    type = TimeDerivative
    variable = temp
  []
  [heat_conduction]
     type = Diffusion
     variable = temp
  []
  [heat_source_fuel]
    type = CoupledForce
    variable = temp
    v = power_density
  []
[]

[BCs]
  [bc]
    type = DirichletBC
    variable = temp
    boundary = '1 3'
    value = 100
  []
  [bc2]
    type = NeumannBC
    variable = temp
    boundary = '0 2'
    value = 10.0
  []
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-7

  end_time = 20
  dt = 2.0
[]

[Postprocessors]
  [temp_fuel_avg]
    type = ElementAverageValue
    variable = temp
    execute_on = 'initial timestep_end'
  []
  [pwr_density]
    type = ElementIntegralVariablePostprocessor
    variable = power_density
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  perf_graph = true
  exodus = true
  color = true
[]
