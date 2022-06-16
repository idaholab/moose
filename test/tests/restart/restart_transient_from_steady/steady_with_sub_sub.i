[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [./power_density]
  [../]
[]

[Variables]
  [./temp]
  [../]
[]

[Kernels]
  [./heat_conduction]
     type = Diffusion
     variable = temp
  [../]
  [./heat_ie]
    type = TimeDerivative
    variable = temp
  [../]
  [./heat_source_fuel]
    type = CoupledForce
    variable = temp
    v = power_density
  [../]
[]

[BCs]
  [bc]
    type = DirichletBC
    variable = temp
    boundary = '0 1 2 3'
    value = 450
  []
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  start_time = 0
  end_time = 10
  dt = 1.0

  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-7
[]

[Postprocessors]
  [./temp_fuel_avg]
    type = ElementAverageValue
    variable = temp
    execute_on = 'initial timestep_end'
  [../]
  [./pwr_density]
    type = ElementIntegralVariablePostprocessor
    variable = power_density
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  perf_graph = true
  exodus = true
  color = true
[]
