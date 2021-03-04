[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmax = 0.5
  ymax = 0.5
[]

[AuxVariables]
  [./power_density]
    family = L2_LAGRANGE
    order = FIRST
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
  type = Steady

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-7
[]

[Postprocessors]
  [./temp_fuel_avg]
    type = ElementAverageValue
    variable = temp
  [../]
  [./pwr_density]
    type = ElementIntegralVariablePostprocessor
    block = '0'
    variable = power_density
    execute_on = 'transfer'
  [../]
[]

[Outputs]
  perf_graph = true
  exodus = true
  color = true
[]
