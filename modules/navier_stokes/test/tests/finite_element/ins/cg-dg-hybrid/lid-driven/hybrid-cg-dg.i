mu = 1
rho = 1
l = 1
U = 1
n = 20

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = ${l}
    nx = ${n}
    ny = ${n}
  []
[]

[Variables]
  [u]
    family = MONOMIAL
  []
  [v]
    family = MONOMIAL
  []
  [pressure][]
[]

[Kernels]
  [momentum_x_convection]
    type = ADConservativeAdvection
    variable = u
    velocity = 'velocity'
    advected_quantity = 'rhou'
  []
  [momentum_x_diffusion]
    type = MatDiffusion
    variable = u
    diffusivity = 'mu'
  []
  [momentum_x_pressure]
    type = PressureGradient
    integrate_p_by_parts = false
    variable = u
    pressure = pressure
    component = 0
  []
  [momentum_x_mass]
    type = MassKernel
    variable = u
    density = ${rho}
    matrix_tags = 'mass'
  []
  [momentum_y_convection]
    type = ADConservativeAdvection
    variable = v
    velocity = 'velocity'
    advected_quantity = 'rhov'
  []
  [momentum_y_diffusion]
    type = MatDiffusion
    variable = v
    diffusivity = 'mu'
  []
  [momentum_y_pressure]
    type = PressureGradient
    integrate_p_by_parts = false
    variable = v
    pressure = pressure
    component = 1
  []
  [momentum_y_mass]
    type = MassKernel
    variable = v
    density = ${rho}
    matrix_tags = 'mass'
  []
  [mass]
    type = ADConservativeAdvection
    variable = pressure
    velocity = velocity
    advected_quantity = -1
  []
[]

[DGKernels]
  [momentum_x_convection]
    type = ADDGAdvection
    variable = u
    velocity = 'velocity'
    advected_quantity = 'rhou'
  []
  [momentum_x_diffusion]
    type = DGDiffusion
    variable = u
    sigma = 6
    epsilon = -1
    diff = 'mu'
  []
  [momentum_y_convection]
    type = ADDGAdvection
    variable = v
    velocity = 'velocity'
    advected_quantity = 'rhov'
  []
  [momentum_y_diffusion]
    type = DGDiffusion
    variable = v
    sigma = 6
    epsilon = -1
    diff = 'mu'
  []
[]

[BCs]
  [u_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left bottom right'
    variable = u
    sigma = 6
    epsilon = -1
    function = '0'
    diff = 'mu'
  []
  [v_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left bottom right top'
    variable = v
    sigma = 6
    epsilon = -1
    function = '0'
    diff = 'mu'
  []
  [u_top]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'top'
    variable = u
    sigma = 6
    epsilon = -1
    function = '${U}'
    diff = 'mu'
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho'
    prop_values = '${rho}'
  []
  [const_reg]
    type = GenericConstantMaterial
    prop_names = 'mu'
    prop_values = '${mu}'
  []
  [vel]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = 'velocity'
    u = u
    v = v
  []
  [rhou]
    type = ADParsedMaterial
    property_name = 'rhou'
    coupled_variables = 'u'
    material_property_names = 'rho'
    expression = 'rho*u'
  []
  [rhov]
    type = ADParsedMaterial
    property_name = 'rhov'
    coupled_variables = 'v'
    material_property_names = 'rho'
    expression = 'rho*v'
  []
[]

[AuxVariables]
  [vel_x]
    family = MONOMIAL
    order = CONSTANT
  []
  [vel_y]
    family = MONOMIAL
    order = CONSTANT
  []
  [p]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [vel_x]
    type = ProjectionAux
    variable = vel_x
    v = u
    execute_on = 'initial timestep_end'
  []
  [vel_y]
    type = ProjectionAux
    variable = vel_y
    v = v
    execute_on = 'initial timestep_end'
  []
  [p]
    type = ProjectionAux
    variable = p
    v = pressure
    execute_on = 'initial timestep_end'
  []
[]

[Problem]
  type = NavierStokesProblem
  mass_matrix = 'mass'
  extra_tag_matrices = 'mass'
[]

[Preconditioning]
  active = FSP
  [FSP]
    type = FSP
    topsplit = 'up'
    [up]
      splitting = 'u p'
      splitting_type  = schur
      petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_type -ksp_pc_side -ksp_rtol'
      petsc_options_value = 'full                            self                              300                fgmres    right        1e-4'
    []
    [u]
      vars = 'u v'
      # petsc_options = '-ksp_monitor'
      petsc_options_iname = '-pc_type -pc_hypre_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side'
      petsc_options_value = 'hypre    boomeramg      gmres     1e-2      300                right'
    []
    [p]
      vars = 'pressure'
      petsc_options = '-pc_lsc_scale_diag -ksp_monitor'# -lsc_ksp_monitor'
      petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side -lsc_pc_type -lsc_ksp_type -lsc_ksp_pc_side -lsc_ksp_rtol'
      petsc_options_value = 'fgmres    300                1e-2      lsc      right        hypre        gmres         right            1e-1'
    []
  []
[]

[UserObjects]
  [set_pressure]
    type = NSFVPressurePin
    pin_type = 'point-value'
    variable = pressure
    point = '0 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    pp_names = ''
    function = '${rho} * ${U} * ${l} / ${mu}'
  []
[]
