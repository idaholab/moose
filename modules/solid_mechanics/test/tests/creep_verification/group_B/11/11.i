[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [cube]
    type = GeneratedMeshGenerator
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
  # Symmetry/support planes
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

  # Applied tensile stresses
  #
  # Pressure uses the outward surface normal. A negative pressure
  # produces an outward tensile traction on these positive faces.
  # Face BCGF: x = L
  # sigma_1 = 300 N/mm^2
  [sigma_1]
    type = Pressure
    variable = disp_x
    component = 0
    boundary = right
    factor = -300.0
    use_displaced_mesh = false
  []

  # Face CDHG: y = L
  # sigma_2 = 200 N/mm^2
  [sigma_2]
    type = Pressure
    variable = disp_y
    component = 1
    boundary = top
    factor = -200.0
    use_displaced_mesh = false
  []

  # Face EFGH: z = L
  # sigma_3 = 100 N/mm^2
  [sigma_3]
    type = Pressure
    variable = disp_z
    component = 2
    boundary = front
    factor = -100.0
    use_displaced_mesh = false
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
    inelastic_models = 'creep'
    tangent_operator = nonlinear
  []

  [creep]
    type = PowerLawCreepStressUpdate

    # Benchmark accumulated-strain law:
    #
    #   eps_c = A * sigma_eff^n * t^m
    #
    #   A = 3.125e-14
    #   n = 5
    #   m = 0.5
    #
    # MOOSE rate-law parameters:
    #
    #   coefficient = A*m
    #   m_exponent  = m-1

    coefficient = 1.5625e-14
    n_exponent = 5.0

    # m_MOOSE = m_benchmark - 1
    m_exponent = -0.5

    activation_energy = 0.0
    start_time = 0.0

    effective_inelastic_strain_name = effective_creep_strain

    max_inelastic_increment = 1e-3
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

  [avg_effective_creep]
    type = ElementAverageValue
    variable = effective_creep_strain
  []

[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  # Time unit: hour
  start_time = 0.0
  end_time = 1000.0

  dtmin = 1e-12
  dtmax = 10.0

  nl_max_its = 50
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12

  automatic_scaling = true

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  # The creep rate is singular as t approaches zero because
  # m_exponent = -0.5. Logarithmic time stepping resolves the
  # rapid initial primary-creep response.
  [TimeSteppers]
    [logarithmic]
      type = LogConstantDT
      first_dt = 1e-8
      log_dt = 0.02
    []
  []
[]

[Outputs]
  exodus = true
  show = "avg_creep_xx avg_creep_yy avg_creep_zz avg_eff_creep"
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
