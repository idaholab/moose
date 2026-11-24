[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3

  xmin = 0.0
  xmax = 100.0
  ymin = 0.0
  ymax = 100.0
  zmin = 0.0
  zmax = 100.0

  nx = 1
  ny = 1
  nz = 1

  elem_type = HEX20
[]

[Functions]
  [coefficientFunction]
    type = ParsedFunction
    expression = 'if(t <= 0, 0, 3.125e-14)'
  []
[]

[Variables]
[]

[AuxVariables]
  [effective_creep_strain]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [effective_creep_strain]
    type = MaterialRealAux
    variable = effective_creep_strain
    property = effective_creep_strain
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = SMALL
    incremental = true

    generate_output = '
      stress_xx stress_yy stress_zz
      stress_xy stress_xz stress_yz
      strain_xx strain_yy strain_zz
      strain_xy strain_xz strain_yz
      creep_strain_xx creep_strain_yy creep_strain_zz
      creep_strain_xy creep_strain_xz creep_strain_yz
    '
  []
[]

[BCs]
  # Face ADEH: x = 0
  [fix_ADEH_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []

  # Face ABFE: y = 0
  [fix_ABFE_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []

  # Face ABCD: z = 0
  [fix_ABCD_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []

  # Face BCGF: x = L
  # u1 = ux = 0.3 mm
  [prescribe_BCGF_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.3
  []

  # Face CDHG: y = L
  # u2 = uy = 0.2 mm
  [prescribe_CDHG_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.2
  []

  # Face EFGH: z = L
  # u3 = uz = 0.1 mm
  [prescribe_EFGH_z]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0.1
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.0e5
    poissons_ratio = 0.3
  []

  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = creep
    tangent_operator = nonlinear
  []

  [creep]
    type = PowerLawCreepStressUpdate

    # Stress unit: N/mm^2
    # Time unit: hour
    coefficient = coefficientFunction
    n_exponent = 5.0
    m_exponent = 0.0
    activation_energy = 0.0
    effective_inelastic_strain_name = effective_creep_strain

    # Limit the creep increment during rapid initial relaxation
    max_inelastic_increment = 1e-5
  []

[]

[Postprocessors]
  # Material-recommended time-step limit
  [material_dt]
    type = MaterialTimeStepPostprocessor
  []

  [avg_eff_creep]
    type = ElementAverageValue
    variable = effective_creep_strain
  []

  # Displacement checks

  [right_disp_x]
    type = SideAverageValue
    variable = disp_x
    boundary = right
  []

  [top_disp_y]
    type = SideAverageValue
    variable = disp_y
    boundary = top
  []

  [front_disp_z]
    type = SideAverageValue
    variable = disp_z
    boundary = front
  []

  # Stress checks
  [avg_stress_xx]
    type = ElementAverageValue
    variable = stress_xx
  []

  [avg_stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  []

  [avg_stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  []

  [avg_stress_xy]
    type = ElementAverageValue
    variable = stress_xy
  []

  [avg_stress_xz]
    type = ElementAverageValue
    variable = stress_xz
  []

  [avg_stress_yz]
    type = ElementAverageValue
    variable = stress_yz
  []

  # Total strain checks
  [avg_strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  []

  [avg_strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  []

  [avg_strain_zz]
    type = ElementAverageValue
    variable = strain_zz
  []

  # Creep strain checks
  [avg_creep_xx]
    type = ElementAverageValue
    variable = creep_strain_xx
  []

  [avg_creep_yy]
    type = ElementAverageValue
    variable = creep_strain_yy
  []

  [avg_creep_zz]
    type = ElementAverageValue
    variable = creep_strain_zz
  []

  [avg_creep_xy]
    type = ElementAverageValue
    variable = creep_strain_xy
  []

  [avg_creep_xz]
    type = ElementAverageValue
    variable = creep_strain_xz
  []

  [avg_creep_yz]
    type = ElementAverageValue
    variable = creep_strain_yz
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  # Time unit is hour.
  start_time = -1.0
  end_time = 1000.0
  # The prescribed displacements are applied in the first increment.
  # Use a small initial step to approximate instantaneous elastic loading.
  dtmin = 1e-10
  dtmax = 10.0

  nl_max_its = 50
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12

  automatic_scaling = true

  [TimeSteppers]
    [adaptive]
      type = IterationAdaptiveDT

      dt = 1
      growth_factor = 1.1 #2.0
      cutback_factor = 0.5

      optimal_iterations = 8
      iteration_window = 2

      timestep_limiting_postprocessor = material_dt

      time_t = '-1 0'
      time_dt = '1 1e-20' # '1 1e-8'
    []
  []
[]

[Outputs]
  exodus = true
  show = "avg_stress_xx avg_stress_yy avg_stress_zz"
  execute_on = 'initial timestep_end'
  [full]
    type = CSV
  []
  [out]
    type = CSV
    sync_times = '0 0.1 1 5 10 50 100 500 1000'
    sync_only = true
  []
[]
