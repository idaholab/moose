[Problem]
  coord_type = RZ
  rz_coord_axis = X
[]

[Mesh]
  file = simple_2D.e
[]

[Functions]
  [./temp]
    type = PiecewiseLinear
    x = '0   1   2'
    y = '100 200 200'
  [../]
[]

[ThermalContact]
  [./thermal_contact]
    type = GapHeatTransfer
    variable = temp
    master = 3
    slave = 2
    gap_geometry_type = CYLINDER
  [../]
[]

[Variables]
  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 100
  [../]
[]

[AuxVariables]
  [./gap_cond]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]


[BCs]
  [./temp_far_left]
    type = FunctionPresetBC
    boundary = 1
    variable = temp
    function = temp
  [../]
  [./temp_far_right]
    type = PresetBC
    boundary = 4
    variable = temp
    value = 100
  [../]
[]

[AuxKernels]
  [./conductance]
    type = MaterialRealAux
    property = gap_conductance
    variable = gap_cond
    boundary = 2
  [../]
[]

[Materials]
  [./heat1]
    type = HeatConductionMaterial
    block = '1 2'
    specific_heat = 1.0
    thermal_conductivity = 100000000.0
  [../]
  [./density]
    type = Density
    block = '1 2'
    density = 1.0
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'
  nl_rel_tol = 1e-12
  l_tol = 1e-3
  l_max_its = 100

  dt = 1e-1
  end_time = 1.0
[]

[Postprocessors]
  [./temp_left]
    type = SideAverageValue
    boundary = 2
    variable = temp
    execute_on = 'initial timestep_end'
  [../]

  [./temp_right]
    type = SideAverageValue
    boundary = 3
    variable = temp
    execute_on = 'initial timestep_end'
  [../]

  [./flux_left]
    type = SideFluxIntegral
    variable = temp
    boundary = 2
    diffusivity = thermal_conductivity
    execute_on = 'initial timestep_end'
  [../]

  [./flux_right]
    type = SideFluxIntegral
    variable = temp
    boundary = 3
    diffusivity = thermal_conductivity
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  exodus = true
[]
