[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    ny = 200
    nx = 10
    ymax = 0.304 # Length of test chamber
    xmax = 0.0257 # Test chamber radius
  []
  coord_type = RZ
[]

[Variables]
  [pressure]
  []
  [temperature]
    initial_condition = 300 # Start at room temperature
  []
[]

[DarcyThermoMech]
[]

[Modules/TensorMechanics/Master]
  [all]
    # This block adds all of the proper Kernels, strain calculators, and Variables
    # for TensorMechanics in the correct coordinate system (autodetected)
    add_variables = true
    strain = FINITE
    eigenstrain_names = eigenstrain
    use_automatic_differentiation = true
    generate_output = 'vonmises_stress elastic_strain_xx elastic_strain_yy strain_xx strain_yy'
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
  viscosity_file = data/water_viscosity.csv
  density_file = data/water_density.csv
  thermal_conductivity_file = data/water_thermal_conductivity.csv
  specific_heat_file = data/water_specific_heat.csv
  thermal_expansion_file = data/water_thermal_expansion.csv

  [column]
    type = PackedColumn
    block = 0
    temperature = temperature
    radius = 1.15
    fluid_viscosity_file = ${viscosity_file}
    fluid_density_file = ${density_file}
    fluid_thermal_conductivity_file = ${thermal_conductivity_file}
    fluid_specific_heat_file = ${specific_heat_file}
    fluid_thermal_expansion_file = ${thermal_expansion_file}
  []

  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 200e9 # (Pa) from wikipedia
    poissons_ratio = .3 # from wikipedia

  []
  [elastic_stress]
    type = ADComputeFiniteStrainElasticStress
  []
  [thermal_strain]
    type = ADComputeThermalExpansionEigenstrain
    stress_free_temperature = 300
    eigenstrain_name = eigenstrain
    temperature = temperature
    thermal_expansion_coeff = 1e-5
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
[]

[Executioner]
  type = Transient
  start_time = -1
  end_time = 200
  steady_state_tolerance = 1e-7
  steady_state_detection = true
  dt = 0.25
  solve_type = PJFNK
  automatic_scaling = true
  compute_scaling_once = false
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 500'
  line_search = none
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
