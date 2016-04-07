[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 200
  ymax = 0.304 # Length of test chamber
  xmax = 0.0257 # Test chamber radius
[]

[Variables]
  [./pressure]
  [../]
  [./temp]
    initial_condition = 300 # Start at room temperature
  [../]
  [./disp_r]
  [../]
  [./disp_z]
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
    variable = temp
  [../]
  [./heat_conduction_time_derivative]
    type = HeatConductionTimeDerivative
    variable = temp
    use_heat_capacity = true
  [../]
  [./heat_convection]
    type = DarcyConvection
    variable = temp
    darcy_pressure = pressure
  [../]
  [./StressDivergence2DAxisymmetricRZ]
    # This block adds all of the proper Kernels for TensorMechanics in RZ
    use_displaced_mesh = true
    displacements = 'disp_r disp_z'
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
    boundary = bottom
    value = 4000 # (Pa) From Figure 2 from paper.  First data point for 1mm balls.
  [../]
  [./outlet]
    type = DirichletBC
    variable = pressure
    boundary = top
    value = 0 # (Pa) Gives the correct pressure drop from Figure 2 for 1mm balls
  [../]
  [./inlet_temperature]
    type = DirichletBC
    variable = temp
    boundary = bottom
    value = 350 # (C)
  [../]
  [./outlet_temperature]
    type = HeatConductionOutflow
    variable = temp
    boundary = top
  [../]
  [./hold_inlet]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
  [../]
  [./hold_center]
    type = DirichletBC
    variable = disp_r
    boundary = left
    value = 0
  [../]
  [./hold_outside]
    type = DirichletBC
    variable = disp_r
    boundary = right
    value = 0
  [../]
[]

[Materials]
  [./column]
    type = PackedColumn
    block = 0
    ball_radius = 1
  [../]

  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200e9 # (Pa) from wikipedia
    poissons_ratio = .3 # from wikipedia
    block = 0
  [../]

  [./small_strain_arz]
    type = ComputeAxisymmetricRZFiniteStrain
    temperature = temp
    thermal_expansion_coeff = 12e-6 # (K^-1) @20C from wikipedia
    block = 0
  [../]

  [./_elastic_strain]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
[]

[Postprocessors]
  [./average_temp]
    type = ElementAverageValue
    variable = temp
  [../]
[]

[Problem]
  type = FEProblem
  coord_type = RZ
[]

[Executioner]
  type = Transient
  num_steps = 50
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 100'
  line_search = none
  nl_rel_tol = 1e-6
  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.1
  [../]
[]

[Outputs]
  exodus = true
[]
