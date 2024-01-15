[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../../step03_boundary_conditions/inputs/mesh_in.e'
  []
[]

# TODO: Add an action for heat conduction

[Modules/IncompressibleNavierStokes]
  # TODO Complete this to set up natural convection in the water regions

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

[Materials]
  # Materials for thermo mechanics
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
