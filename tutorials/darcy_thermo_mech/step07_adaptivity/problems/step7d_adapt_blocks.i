[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 4
  xmax = 0.304 # Length of test chamber
  ymax = 0.0257 # Test chamber radius
  uniform_refine = 3
[]

[MeshModifiers]
  [./bottom]
    type = SubdomainBoundingBox
    location = inside
    bottom_left = '0 0 0'
    top_right = '0.304 0.01285 0'
    block_id = 1
  [../]
[]

[Variables]
  [./pressure]
  [../]
  [./temperature]
    initial_condition = 300 # Start at room temperature
  [../]
[]

[AuxVariables]
  [./velocity_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./velocity_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./velocity_z]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./darcy_pressure]
    type = DarcyPressure
    variable = pressure
  [../]
  [./heat_conduction]
    type = HeatConduction
    variable = temperature
  [../]
  [./heat_conduction_time_derivative]
    type = HeatCapacityConductionTimeDerivative
    variable = temperature
  [../]
  [./heat_convection]
    type = DarcyConvection
    variable = temperature
    darcy_pressure = pressure
  [../]
[]

[AuxKernels]
  [./velocity_x]
    type = DarcyVelocity
    variable = velocity_x
    component = x
    execute_on = timestep_end
    darcy_pressure = pressure
  [../]
  [./velocity_y]
    type = DarcyVelocity
    variable = velocity_y
    component = y
    execute_on = timestep_end
    darcy_pressure = pressure
  [../]
  [./velocity_z]
    type = DarcyVelocity
    variable = velocity_z
    component = z
    execute_on = timestep_end
    darcy_pressure = pressure
  [../]
[]

[BCs]
  [./inlet]
    type = DirichletBC
    variable = pressure
    boundary = left
    value = 4000 # (Pa) From Figure 2 from paper.  First data point for 1mm spheres.
  [../]
  [./outlet]
    type = DirichletBC
    variable = pressure
    boundary = right
    value = 0 # (Pa) Gives the correct pressure drop from Figure 2 for 1mm spheres
  [../]
  [./inlet_temperature]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 350 # (C)
  [../]
  [./outlet_temperature]
    type = HeatConductionBC
    variable = temperature
    boundary = right
  [../]
[]

[Materials]
  [./column_bottom]
    type = PackedColumn
    block = 1
    sphere_radius = 1.5
  [../]
  [./column_top]
    type = PackedColumn
    block = 0
    sphere_radius = 1
  [../]
[]

[Problem]
  type = FEProblem
  coord_type = RZ
  rz_coord_axis = X
[]

[Executioner]
  type = Transient
  dt = 0.01
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  end_time = 10
  [./TimeStepper]
    type = IterationAdaptiveDT
    linear_iteration_ratio = 5
    cutback_factor = 0.4
    dt = 0.01
    growth_factor = 1.2
    optimal_iterations = 4
  [../]
[]

[Adaptivity]
  marker = error_frac
  max_h_level = 3
  [./Indicators]
    [./temperature_jump]
      type = GradientJumpIndicator
      variable = temperature
      scale_by_flux_faces = true
    [../]
  [../]
  [./Markers]
    [./error_frac]
      type = ErrorFractionMarker
      coarsen = 0.1
      indicator = temperature_jump
      refine = 0.6
    [../]
  [../]
[]

[Outputs]
  [./exodus]
    type = Exodus
    output_material_properties = true
  [../]
[]
