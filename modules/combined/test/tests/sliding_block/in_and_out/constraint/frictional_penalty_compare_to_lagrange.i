# This input file is meant for direct performance comparison with frictional_lagrange.i

[Mesh]
  file = sliding_elastic_blocks_2d_with_lagrange.e
  patch_size = 80
[]

[Problem]
  kernel_coverage_check = false
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
    block = '1 2'
  [../]
  [./disp_y]
    block = '1 2'
  [../]
  [./vel_x]
    block = '2'
  [../]
  [./vel_y]
    block = '2'
  [../]
[]

[AuxVariables]
  [./penetration]
  [../]
  [./inc_slip_x]
  [../]
  [./inc_slip_y]
  [../]
  [./accum_slip_x]
  [../]
  [./accum_slip_y]
  [../]
[]

[Functions]
  [./vertical_movement]
    type = ParsedFunction
    value = -t
  [../]
  [./horizontal_movement]
    type = ParsedFunction
    value = -0.04*sin(4*t)+0.02
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    incremental = false
    add_variables = true
    generate_output = 'strain_xx strain_yy strain_zz' ## Not at all necessary, but nice
    block = '1 2'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    block = '1 2'
  [../]
  [./small_stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  [../]
  [./dummy]
    type = GenericConstantMaterial
    prop_names = 'dumb'
    prop_values = '0'
    block = 30
  [../]
[]

[Kernels]
  [./accel_x]
    type = CoupledTimeDerivative
    variable = disp_x
    v = vel_x
    block = 2
  [../]
  [./accel_y]
    type = CoupledTimeDerivative
    variable = disp_y
    v = vel_y
    block = 2
  [../]
  [./coupled_time_velx]
    type = CoupledTimeDerivative
    variable = vel_x
    v = disp_x
    block = 2
  [../]
  [./coupled_time_vely]
    type = CoupledTimeDerivative
    variable = vel_y
    v = disp_y
    block = 2
  [../]
  [./source_velx]
    type = MatReaction
    variable = vel_x
    mob_name = 1
    block = 2
  [../]
  [./source_vely]
    type = MatReaction
    variable = vel_y
    mob_name = 1
    block = 2
  [../]
[]

[AuxKernels]
  [./zeroslip_x]
    type = ConstantAux
    variable = inc_slip_x
    boundary = 3
    execute_on = timestep_begin
    value = 0.0
  [../]
  [./zeroslip_y]
    type = ConstantAux
    variable = inc_slip_y
    boundary = 3
    execute_on = timestep_begin
    value = 0.0
  [../]
  [./accum_slip_x]
    type = AccumulateAux
    variable = accum_slip_x
    accumulate_from_variable = inc_slip_x
    execute_on = timestep_end
  [../]
  [./accum_slip_y]
    type = AccumulateAux
    variable = accum_slip_y
    accumulate_from_variable = inc_slip_y
    execute_on = timestep_end
  [../]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 3
    paired_boundary = 2
  [../]
[]

[Postprocessors]
  [./nonlinear_its]
    type = NumNonlinearIterations
    execute_on = timestep_end
  [../]
  [./tot_nonlinear_its]
    type = CumulativeValuePostprocessor
    postprocessor = nonlinear_its
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./right_x]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 4
    function = horizontal_movement
  [../]
  [./right_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 4
    function = vertical_movement
  [../]
[]


[Preconditioning]
  [./smp]
     type = SMP
     full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-options_left'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -mat_mffd_err -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = '30                 lu       1e-5          NONZERO               1e-15'
  line_search = basic

  l_max_its = 30
  dt = 0.1
  end_time = 3
  dtmin = 0.01
  nl_max_its = 10
[]

[Outputs]
  # checkpoint = true
  print_linear_residuals = true
  print_perf_log = true
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
  [./dof_map]
    execute_on = 'initial'
    type = DOFMap
  [../]
[]

[Contact]
  [./leftright]
    slave = 3
    master = 2
    model = coulomb
    formulation = penalty
    penalty = 1e7
    friction_coefficient = 0.4
    normal_smoothing_distance = 0.1
    system = constraint
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]
