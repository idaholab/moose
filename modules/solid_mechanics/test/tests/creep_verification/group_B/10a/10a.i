# Units: N, mm, hour

[GlobalParams]
  displacements = 'disp_x disp_y'
  out_of_plane_strain = strain_zz
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4

    xmin = 0.0
    xmax = 100.0
    ymin = 0.0
    ymax = 100.0

    elem_type = QUAD8
  []
[]

[Variables]
  # Required for WEAK_PLANE_STRESS
  [strain_zz]
    order = SECOND
    family = LAGRANGE
  []
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
    planar_formulation = WEAK_PLANE_STRESS
    use_automatic_differentiation = false

    generate_output = 'stress_xx stress_yy stress_xy stress_zz strain_xx strain_yy strain_xy creep_strain_xx creep_strain_yy creep_strain_zz creep_strain_xy'
  []
[]

[BCs]
  # Line AD: prescribed zero x-displacement
  [fix_x_on_AD]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []

  # Line AB: zero y displacement
  [fix_y_on_AB]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []

  # BC: tensile sigma_1 = +200 N/mm^2
  #
  # Pressure is positive inward, so a negative value applies
  # outward tension on the right boundary.
  [right_tension]
    type = NeumannBC
    variable = disp_x
    boundary = right
    value = 200
    use_displaced_mesh = false
  []

  # CD: compressive sigma_2 = -200 N/mm^2
  #
  # Positive pressure on the top boundary acts downward.
  [top_compression]
    type = NeumannBC
    variable = disp_y
    boundary = top
    value = -200
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

    # A_MOOSE = A_benchmark*m_benchmark
    coefficient = 1.5625e-14
    n_exponent = 5.0

    # m_MOOSE = m_benchmark - 1
    m_exponent = -0.5

    activation_energy = 0.0

    relative_tolerance = 1e-12
    absolute_tolerance = 1e-14
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

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  # Time unit: hour
  start_time = 0.0
  end_time = 1000.0

  nl_max_its = 50
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  l_tol = 1e-10

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
