[GlobalParams]
  displacements = 'disp_x disp_y'
  out_of_plane_strain = strain_zz
[]

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2

    xmin = 0.0
    xmax = 100.0
    ymin = 0.0
    ymax = 100.0

    nx = 1
    ny = 1
    elem_type = QUAD8
  []

  # Point A = (0,0)
  [point_A]
    type = ExtraNodesetGenerator
    input = generated
    new_boundary = point_A
    coord = '0.0 0.0 0.0'
  []

  # Point B = (L,0)
  [point_B]
    type = ExtraNodesetGenerator
    input = point_A
    new_boundary = point_B
    coord = '100.0 0.0 0.0'
  []
[]

[Variables]
  # Nonlinear out-of-plane strain variable required by
  # the weak plane-stress formulation
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
    type = ADMaterialRealAux
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
    use_automatic_differentiation = true

    # Do not include strain_zz here because it is already
    # defined as the nonlinear plane-stress variable.
    generate_output = '
      stress_xx stress_yy stress_xy stress_zz
      strain_xx strain_yy strain_xy
      creep_strain_xx creep_strain_yy
      creep_strain_zz creep_strain_xy
    '
  []
[]

[BCs]
  # Point A: ux = 0, uy = 0
  [fix_A_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = point_A
    value = 0.0
  []

  [fix_A_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = point_A
    value = 0.0
  []

  # Point B: uy = 0
  [fix_B_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = point_B
    value = 0.0
  []

  # Constant shear traction tau = 100 N/mm^2.
  #
  # AB: leftward
  [shear_AB]
    type = ADNeumannBC
    variable = disp_x
    boundary = bottom
    value = -100.0
  []

  # BC: upward
  [shear_BC]
    type = ADNeumannBC
    variable = disp_y
    boundary = right
    value = 100.0
  []

  # CD: rightward
  [shear_CD]
    type = ADNeumannBC
    variable = disp_x
    boundary = top
    value = 100.0
  []

  # AD: downward
  [shear_AD]
    type = ADNeumannBC
    variable = disp_y
    boundary = left
    value = -100.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 2.0e5
    poissons_ratio = 0.3
  []

  [stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = 'creep'
    # Appropriate for incremental small-strain calculations
    perform_finite_strain_rotations = false
  []

  [creep]
    type = ADPowerLawCreepStressUpdate

    # Units:
    # stress = N/mm^2
    # time   = hour
    coefficient = 3.125e-14
    n_exponent = 5.0
    m_exponent = 0.0
    activation_energy = 0.0
    effective_inelastic_strain_name = effective_creep_strain
  []

[]

[Postprocessors]
  [avg_eff_creep]
    type = ElementAverageValue
    variable = effective_creep_strain
  []


  # Displacement check on loaded edge
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

  # Total strain checks
  [avg_strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  []

  [avg_strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  []

  # MOOSE tensor shear strain epsilon_xy
  [avg_strain_xy_tensor]
    type = ElementAverageValue
    variable = strain_xy
  []

  # Engineering total shear strain gamma_xy = 2 epsilon_xy
  [avg_gamma_xy]
    type = ScalePostprocessor
    value = avg_strain_xy_tensor
    scaling_factor = 2.0
  []

  # The nonlinear plane-stress variable
  [avg_strain_zz]
    type = ElementAverageValue
    variable = strain_zz
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

  # MOOSE tensor creep shear strain epsilon^c_xy
  [avg_creep_xy_tensor]
    type = ElementAverageValue
    variable = creep_strain_xy
  []

  # NAFEMS reports engineering creep shear strain:
  # gamma^c_xy = 2 epsilon^c_xy
  [avg_creep_gamma_xy]
    type = ScalePostprocessor
    value = avg_creep_xy_tensor
    scaling_factor = 2.0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  start_time = 0.0
  end_time = 1000.0
  dt = 1.0

  nl_max_its = 50
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12

  automatic_scaling = true
[]

[Outputs]
  exodus = true
  show = 'avg_creep_gamma_xy'
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
