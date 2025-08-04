starting_point = 2e-1
offset = -0.19

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = long-bottom-block-no-lower-d.e
  []
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
[]

[ICs]
  [disp_y]
    block = 2
    variable = disp_y
    value = '${fparse starting_point + offset}'
    type = ConstantIC
  []
[]

[Kernels]
  [DynamicTensorMechanics]
    displacements = 'disp_x disp_y'
    generate_output = 'stress_xx stress_yy'
    strain = FINITE
    block = '1 2'
    stiffness_damping_coefficient = 0.05
    hht_alpha = 0.0
  []
  [inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.25
    gamma = 0.5
    alpha = 0
    eta = 0.0
    block = '1 2'
  []
  [inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.25
    gamma = 0.5
    alpha = 0
    eta = 0.0
    block = '1 2'
  []
[]

[Materials]
  [elasticity_2]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [elasticity_1]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e8
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  []
  [strain]
    type = ComputeFiniteStrain
    block = '1 2'
  []
  [density]
    type = GenericConstantMaterial
    block = '1 2'
    prop_names = 'density'
    prop_values = '7750'
  []
[]

[AuxVariables]
  [vel_x]
    block = '1 2'
  []
  [accel_x]
    block = '1 2'
  []
  [vel_y]
    block = '1 2'
  []
  [accel_y]
    block = '1 2'
  []
  [vel_z]
    block = '1 2'
  []
  [accel_z]
    block = '1 2'
  []
[]

[AuxKernels]
  [accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.25
    execute_on = 'LINEAR timestep_end'
  []
  [vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    gamma = 0.5
    execute_on = 'LINEAR timestep_end'
  []
  [accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    beta = 0.25
    execute_on = 'LINEAR timestep_end'
  []
  [vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    gamma = 0.5
    execute_on = 'LINEAR timestep_end'
  []
[]

[Contact]
  [mechanical]
    formulation = mortar
    model = coulomb
    primary = 20
    secondary = 10
    friction_coefficient = 0.5
    c_normal = 1.0e4
    c_tangential = 1.0e4
    mortar_dynamics = true
    newmark_beta = 0.25
    newmark_gamma = 0.5
    capture_tolerance = 1.0e-5
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  []
  [topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = '${starting_point} * cos(2 * pi / 4 * t) + ${offset}'
  []
  [leftx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 30 # 50
    function = '0' # '1e-2*t'
  []
[]

[Executioner]
  type = Transient
  end_time = 75
  dt = 0.05
  dtmin = .005
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor '
                  '-snes_linesearch_monitor -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -mat_mffd_err '
  petsc_options_value = 'lu       NONZERO               1e-15                   1e-5'
  nl_max_its = 50
  line_search = 'none'
  snesmf_reuse_base = false

  [TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
[]

[VectorPostprocessors]
  [mechanical_tangential_lm]
    type = NodalValueSampler
    block = 'mechanical_secondary_subdomain'
    variable = mechanical_tangential_lm
    sort_by = 'x'
    execute_on = TIMESTEP_END
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
