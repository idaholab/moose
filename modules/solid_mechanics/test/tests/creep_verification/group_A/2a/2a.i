[GlobalParams]
  displacements = 'disp_x disp_y'
  out_of_plane_strain = strain_zz
[]

[Mesh]
  type = GeneratedMesh
  dim = 2

  xmin = 0.0
  xmax = 100.0
  ymin = 0.0
  ymax = 100.0

  nx = 1
  ny = 1
  elem_type = QUAD8
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

    generate_output = 'stress_xx stress_yy stress_xy stress_zz strain_xx strain_yy strain_xy creep_strain_xx creep_strain_yy creep_strain_zz creep_strain_xy'
  []
[]

[BCs]
  # Line AD: prescribed zero x-displacement
  [fix_x_on_AD]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []

  # Line AB: prescribed zero y-displacement
  [fix_bottom_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []

  # Line BC: prescribed tensile stress sigma_1
  [right_tension]
    type = ADNeumannBC
    variable = disp_x
    boundary = right
    value = 200.0
  []

  # Line CD: prescribed tensile stress sigma_2
  [top_tension]
    type = ADNeumannBC
    variable = disp_y
    boundary = top
    value = 200.0
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
  []

  [creep]
    type = ADPowerLawCreepStressUpdate
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
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  start_time = 0.0
  end_time = 1000.0
  dt = 1.0

  l_max_its = 100
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12

  automatic_scaling = true
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
