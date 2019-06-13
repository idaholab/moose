[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  ny = 200
  nx = 10
  ymax = 0.304 # Length of test chamber
  xmax = 0.0257 # Test chamber radius
[]

[MeshModifiers]
  [bottom]
    type = SubdomainBoundingBox
    location = inside
    bottom_left = '0 0 0'
    top_right = '0.01285 0.304 0'
    block_id = 1
  []
[]

[Variables]
  [pressure]
  []
  [temperature]
    initial_condition = 300 # Start at room temperature
  []
[]

[AuxVariables]
  [velocity_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [velocity_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [velocity_z]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    # This block adds all of the proper Kernels, strain calculators, and Variables
    # for TensorMechanics in the correct coordinate system (autodetected)
    add_variables = true
    strain = FINITE
    eigenstrain_names = eigenstrain
    generate_output = 'vonmises_stress elastic_strain_xx elastic_strain_yy strain_xx strain_yy'
  []
[]

[Kernels]
  [darcy_pressure]
    type = DarcyPressure
    variable = pressure
  []
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
  [heat_conduction_time_derivative]
    type = ADHeatConductionTimeDerivative
    variable = temperature
  []
  [heat_convection]
    type = DarcyAdvection
    variable = temperature
    pressure = pressure
  []
[]

[AuxKernels]
  [velocity_x]
    type = DarcyVelocity
    variable = velocity_x
    component = x
    execute_on = timestep_end
    pressure = pressure
  []
  [velocity_y]
    type = DarcyVelocity
    variable = velocity_y
    component = y
    execute_on = timestep_end
    pressure = pressure
  []
  [velocity_z]
    type = DarcyVelocity
    variable = velocity_z
    component = z
    execute_on = timestep_end
    pressure = pressure
  []
[]

[BCs]
  [inlet]
    type = DirichletBC
    variable = pressure
    boundary = bottom
    value = 4000 # (Pa) From Figure 2 from paper.  First data point for 1mm spheres.
  []
  [outlet]
    type = DirichletBC
    variable = pressure
    boundary = top
    value = 0 # (Pa) Gives the correct pressure drop from Figure 2 for 1mm spheres
  []
  [inlet_temperature]
    type = FunctionDirichletBC
    variable = temperature
    boundary = bottom
    function = 'if(t<0,350+50*t,350)'
  []
  [outlet_temperature]
    type = HeatConductionOutflow
    variable = temperature
    boundary = top
  []
  [hold_inlet]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
  []
  [hold_center]
    type = DirichletBC
    variable = disp_r
    boundary = left
    value = 0
  []
  [hold_outside]
    type = DirichletBC
    variable = disp_r
    boundary = right
    value = 0
  []
[]

[Materials]
  [column_top]
    type = PackedColumn
    block = 0
    temperature = temperature
    radius = 1.15
  []
  [column_bottom]
    type = PackedColumn
    block = 1
    temperature = temperature
    radius = 1
  []

  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200e9 # (Pa) from wikipedia
    poissons_ratio = .3 # from wikipedia

  []
  [elastic_stress]
    type = ComputeFiniteStrainElasticStress
  []
  [thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 300
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain
    temperature = temperature
  []
[]

[Postprocessors]
  [average_temperature]
    type = ElementAverageValue
    variable = temperature
  []
[]

[Problem]
  type = FEProblem
  coord_type = RZ
[]

[Executioner]
  type = Transient
  start_time = -1
  end_time = 200
  steady_state_tolerance = 1e-7
  steady_state_detection = true
  dt = 0.25
  #solve_type = NEWTON
  #petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_factor_levels'
  #petsc_options_value = 'asm      3               9'
  #l_tol = 1e-9
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 100'
  line_search = none
  nl_rel_tol = 1e-7
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0,0.1,0.25)'
  []
[]

[Outputs]
  [out]
    type = Exodus
    elemental_as_nodal = true
  []
[]
