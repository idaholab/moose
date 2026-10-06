[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
    nx = 2
    ny = 1
    nz = 1
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '0 0 0'
    top_right = '1 1 1'
    block_id = 1
    block_name = left
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = left
    bottom_left = '1 0 0'
    top_right = '2 1 1'
    block_id = 2
    block_name = right
  []
  [split]
    type = BreakMeshByBlockGenerator
    input = right
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
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
        boundary = left_right
        strain = SMALL
        generate_output = 'traction_x traction_y traction_z'
      []
    []
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0
  []
  [left_y]
    type = DirichletBC
    boundary = left
    variable = disp_y
    value = 0
  []
  [left_z]
    type = DirichletBC
    boundary = left
    variable = disp_z
    value = 0
  []
  [right_x]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_x
    function = '-1e-8*t'
  []
  [right_y]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_y
    function = '3*t'
  []
  [right_z]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_z
    function = '0*t'
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
  [jump_tangential_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [jump_tangential_2]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [delta_final]
    type = MaterialRealAux
    boundary = left_right
    variable = delta_final
    property = effective_displacement_jump_at_full_degradation
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [jump_normal]
    type = MaterialRealVectorValueAux
    boundary = left_right
    variable = jump_normal
    property = interface_displacement_jump
    component = 0
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [jump_tangential_1]
    type = MaterialRealVectorValueAux
    boundary = left_right
    variable = jump_tangential_1
    property = interface_displacement_jump
    component = 1
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [jump_tangential_2]
    type = MaterialRealVectorValueAux
    boundary = left_right
    variable = jump_tangential_2
    property = interface_displacement_jump
    component = 2
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
    boundary = left_right
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
  [traction_y]
    type = SideAverageValue
    boundary = left_right
    variable = traction_y
  []
  [traction_z]
    type = SideAverageValue
    boundary = left_right
    variable = traction_z
  []
  [shear_traction_magnitude]
    type = ParsedPostprocessor
    expression = 'sqrt(traction_y*traction_y + traction_z*traction_z)'
    pp_names = 'traction_y traction_z'
  []
  [energy_rate]
    type = ParsedPostprocessor
    expression = 'abs(traction_y)*3'
    pp_names = 'traction_y'
  []
  [numerical_energy]
    type = TimeIntegratedPostprocessor
    value = energy_rate
    time_integration_scheme = trapezoidal-rule
  []
  [prescribed_energy]
    type = FunctionValuePostprocessor
    function = 2
  []
  [relative_error]
    type = ParsedPostprocessor
    expression = 'abs(numerical_energy-prescribed_energy)/prescribed_energy'
    pp_names = 'numerical_energy prescribed_energy'
  []
  [final_separation]
    type = SideAverageValue
    boundary = left_right
    variable = delta_final
  []
  [tangential_jump]
    type = ParsedPostprocessor
    expression = 'sqrt(jump_t1*jump_t1 + jump_t2*jump_t2)'
    pp_names = 'jump_t1 jump_t2'
  []
  [jump_t1]
    type = SideAverageValue
    boundary = left_right
    variable = jump_tangential_1
  []
  [jump_t2]
    type = SideAverageValue
    boundary = left_right
    variable = jump_tangential_2
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
    # Both shear directions have speed 3: initiation at (1/100)/3 and failure at 4/3.
    sync_times = '0.00333333333333333 1.33333333333333'
    show = 'prescribed_energy numerical_energy relative_error final_separation bk_mode_II_limit pure_shear_to_bk_limit_ratio'
  []
[]
