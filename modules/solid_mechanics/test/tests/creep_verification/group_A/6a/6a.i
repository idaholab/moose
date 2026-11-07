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

[Modules/TensorMechanics/Master]
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
  # Face ADEH: ux = 0
  [fix_left_x]
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
  # sigma_1 = 300 N/mm^2
  [sigma_1]
    type = NeumannBC
    variable = disp_x
    boundary = right
    value = 300.0
  []

  # Face CDHG: y = L
  # sigma_2 = 200 N/mm^2
  [sigma_2]
    type = NeumannBC
    variable = disp_y
    boundary = top
    value = 200.0
  []

  # Face EFGH: z = L
  # sigma_3 = 100 N/mm^2
  [sigma_3]
    type = NeumannBC
    variable = disp_z
    boundary = front
    value = 100.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200e3
    poissons_ratio = 0.3
  []

  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = creep
  []

  [creep]
    type = PowerLawCreepStressUpdate

    # Stress unit: N/mm^2
    # Time unit: hour
    coefficient = 3.125e-14
    n_exponent = 5.0
    m_exponent = 0.0
    activation_energy = 0.0
    effective_inelastic_strain_name = effective_creep_strain

    # At the prescribed stress, the effective creep-strain increment
    # is approximately 0.00487 per hour. This avoids unnecessary
    # time-step cutbacks from the default 1e-4 limit.
    max_inelastic_increment = 0.01
  []

[]

[Postprocessors]
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
  show = 'avg_creep_xx avg_creep_yy avg_creep_zz avg_eff_creep'
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
