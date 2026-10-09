[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymax = 2
    nx = 1
    ny = 2
  []
  [lower]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
    block_name = lower
  []
  [upper]
    type = SubdomainBoundingBoxGenerator
    input = lower
    bottom_left = '0 1 0'
    top_right = '1 2 0'
    block_id = 2
    block_name = upper
  []
  [split]
    type = BreakMeshByBlockGenerator
    input = upper
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        add_variables = true
        strain = SMALL
      []
    []
    [CohesiveZone]
      [czm]
        boundary = lower_upper
        strain = SMALL
        generate_output = 'traction_x traction_y'
      []
    []
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
  []
  [top_x]
    type = FunctionDirichletBC
    boundary = top
    variable = disp_x
    function = '0*t'
  []
  [top_y]
    type = FunctionDirichletBC
    boundary = top
    variable = disp_y
    function = '3*t'
  []
[]

[AuxVariables]
  [delta_final]
    order = CONSTANT
    family = MONOMIAL
  []
  [jump_normal]
    order = CONSTANT
    family = MONOMIAL
  []
  [jump_tangential]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [delta_final]
    type = MaterialRealAux
    boundary = lower_upper
    variable = delta_final
    property = effective_displacement_jump_at_full_degradation
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [jump_normal]
    type = MaterialRealVectorValueAux
    boundary = lower_upper
    variable = jump_normal
    property = interface_displacement_jump
    component = 0
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [jump_tangential]
    type = MaterialRealVectorValueAux
    boundary = lower_upper
    variable = jump_tangential
    property = interface_displacement_jump
    component = 1
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e12
    poissons_ratio = 0
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [czm]
    type = BiLinearMixedModeTraction
    boundary = lower_upper
    penalty_stiffness = 100
    GI_c = 1
    GII_c = 2
    normal_strength = 1
    shear_strength = 1
    eta = 2
    mixed_mode_criterion = BK
  []
[]

[Postprocessors]
  [traction_x]
    type = SideAverageValue
    boundary = lower_upper
    variable = traction_x
  []
  [traction_y]
    type = SideAverageValue
    boundary = lower_upper
    variable = traction_y
  []
  [normal_traction]
    type = SideAverageValue
    boundary = lower_upper
    variable = traction_y
  []
  [tangential_traction]
    type = SideAverageValue
    boundary = lower_upper
    variable = traction_x
  []
  [energy_rate]
    type = ParsedPostprocessor
    expression = 'abs(traction_y)*3'
    pp_names = 'traction_x traction_y'
  []
  [numerical_energy]
    type = TimeIntegratedPostprocessor
    value = energy_rate
    time_integration_scheme = trapezoidal-rule
  []
  [prescribed_energy]
    type = FunctionValuePostprocessor
    function = 1
  []
  [relative_error]
    type = ParsedPostprocessor
    expression = 'abs(numerical_energy-prescribed_energy)/prescribed_energy'
    pp_names = 'numerical_energy prescribed_energy'
  []
  [final_separation]
    type = SideAverageValue
    boundary = lower_upper
    variable = delta_final
  []
  [normal_separation]
    type = SideAverageValue
    boundary = lower_upper
    variable = jump_normal
  []
  [tangential_separation]
    type = SideAverageValue
    boundary = lower_upper
    variable = jump_tangential
  []
  [bk_mode_II_limit]
    type = FunctionValuePostprocessor
    function = 4
  []
  [pure_shear_to_bk_limit_ratio]
    type = ParsedPostprocessor
    expression = 'final_separation/bk_mode_II_limit'
    pp_names = 'final_separation bk_mode_II_limit'
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = lu
  nl_abs_tol = 1e-10
  dt = 0.2
  end_time = 2
[]

[Outputs]
  console = false
  [csv]
    type = CSV
    execute_on = FINAL
    # Resolve the traction-law corners so trapezoidal integration can use coarse steps.
    # Initiation times: (1/100)/(3*sqrt(2)) for mixed mode and (1/100)/3 for pure modes.
    # Failure times: 2.5/(3*sqrt(2)), 2/3, and 4/3 for mixed mode, mode I, and mode II.
    sync_times = '0.00235702260395516 0.00333333333333333 0.58925565098879 0.666666666666667 1.33333333333333'
    show = 'prescribed_energy numerical_energy relative_error final_separation bk_mode_II_limit pure_shear_to_bk_limit_ratio'
  []
[]
