[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  # Required for NodalVariableValue on distributed mesh
  allow_renumbering = false
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [p]
    initial_condition = 1e5
  []
  [T]
    initial_condition = 700
  []
  [rho]
  []
[]

[FluidProperties]
  [salt]
    type = SalineMoltenSaltFluidProperties
    comp_name = "LiF NaF KF"
    comp_val = "0.465 0.115 0.42"
    prop_def_file = "saline_custom.prp"
  []
[]

[AuxKernels]
  [rho_aux]
    type = FluidDensityAux
    variable = rho
    p = p
    T = T
    fp = salt
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [rho]
    type = NodalVariableValue
    variable = rho
    nodeid = 0
  []
  # Acceptance test
  [check_rho]
    type = PostprocessorComparison
    comparison_type = "equals"
    value_a = '${fparse (2.579-6.24e-4*700)*1000}'
    value_b = "rho"
    absolute_tolerance = "0.1" # kg/m^3
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
