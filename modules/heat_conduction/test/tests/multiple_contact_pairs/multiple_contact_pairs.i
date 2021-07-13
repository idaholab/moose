[Mesh]
  file = 3blk.e
[]

[Functions]
  [temperature]
    type = PiecewiseLinear
    x = '0   1   2'
    y = '100 300 300'
  []
[]

[ThermalContact]
  [thermal_contact]
    type = GapHeatTransfer
    variable = temperature
    primary = '101 201'
    secondary = '100 200'
    emissivity_primary = 0
    emissivity_secondary = 0
    gap_conductance = 1.0e9
  []
[]

[Variables]
  [temperature]
    order = FIRST
    family = LAGRANGE
    initial_condition = 100
  []
[]

[AuxVariables]
  [gap_cond]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = temperature
  []
[]

[BCs]
  [temp_far_left]
    type = FunctionDirichletBC
    boundary = '101 201'
    variable = temperature
    function = temperature
  []
  [temp_far_right]
    type = DirichletBC
    boundary = 'left right'
    variable = temperature
    value = 100
  []
[]

[AuxKernels]
  [conductance]
    type = MaterialRealAux
    property = gap_conductance
    variable = gap_cond
    boundary = 100
  []
[]

[Materials]
  [heat1]
    type = HeatConductionMaterial
    block = '1 2 3'
    specific_heat = 1.0
    thermal_conductivity = 100000000.0
  []
  [density]
    type = GenericConstantMaterial
    block = '1 2 3'
    prop_names = 'density'
    prop_values = '1.0'
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'
  nl_rel_tol = 1e-8
  l_tol = 1e-3
  l_max_its = 100

  dt = 1e-1
  end_time = 1.0
[]

[Postprocessors]
  [temp_left]
    type = SideAverageValue
    boundary = 100
    variable = temperature
    execute_on = 'initial timestep_end'
  []

  [temp_right]
    type = SideAverageValue
    boundary = 200
    variable = temperature
    execute_on = 'initial timestep_end'
  []

  [flux_left]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    boundary = 100
    diffusivity = thermal_conductivity
    execute_on = 'initial timestep_end'
  []

  [flux_right]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    boundary = 200
    diffusivity = thermal_conductivity
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  exodus = false
  csv = true
[]
