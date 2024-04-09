mu = 1
rho = 1
l = 1
U = 1
n = 8
gamma = ${U}
omega_q = 1
omega_m = 1

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
    elem_type = QUAD9
  []
[]

[Problem]
  type = NavierStokesProblem
  extra_tag_matrices = 'mass'
  mass_matrix = 'mass'
  use_pressure_mass_matrix = true
[]

[Variables]
  [vel_x]
    family = L2_HIERARCHIC
    order = FIRST
  []
  [vel_y]
    family = L2_HIERARCHIC
    order = FIRST
  []
  [pressure]
    family = MONOMIAL
    order = CONSTANT
  []
  [vel_bar_x]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
  [vel_bar_y]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
  [pressure_bar]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
[]

[Kernels]
  [momentum_x_convection]
    type = ADConservativeAdvection
    variable = vel_x
    velocity = 'velocity'
    advected_quantity = 'rhou'
  []
  [momentum_x_diffusion]
    type = ADMatDiffusion
    variable = vel_x
    diffusivity = 'mu'
  []
  [momentum_x_pressure]
    type = PressureGradient
    integrate_p_by_parts = true
    variable = vel_x
    pressure = pressure
    component = 0
  []

  [momentum_y_convection]
    type = ADConservativeAdvection
    variable = vel_y
    velocity = 'velocity'
    advected_quantity = 'rhov'
  []
  [momentum_y_diffusion]
    type = ADMatDiffusion
    variable = vel_y
    diffusivity = 'mu'
  []
  [momentum_y_pressure]
    type = PressureGradient
    integrate_p_by_parts = true
    variable = vel_y
    pressure = pressure
    component = 1
  []

  [mass]
    type = ADConservativeAdvection
    variable = pressure
    velocity = velocity
    advected_quantity = ${fparse -rho}
  []

  [mass_matrix_pressure]
    type = MassMatrix
    variable = pressure
    matrix_tags = 'mass'
    density = ${fparse -gamma * omega_q}
  []

  # [grad_div_x]
  #   type = GradDiv
  #   variable = vel_x
  #   u = vel_x
  #   v = vel_y
  #   gamma = ${gamma}
  #   component = 0
  # []
  # [grad_div_y]
  #   type = GradDiv
  #   variable = vel_y
  #   u = vel_x
  #   v = vel_y
  #   gamma = ${gamma}
  #   component = 1
  # []
[]

[DGKernels]
  [momentum_x_convection]
    type = ADHDGAdvection
    variable = vel_x
    velocity = 'velocity'
    coeff = ${rho}
    side_variable = vel_bar_x
  []
  [momentum_x_convection_side]
    type = ADHDGAdvectionSide
    variable = vel_bar_x
    velocity = 'velocity'
    coeff = ${rho}
    interior_variable = vel_x
  []
  [momentum_x_diffusion]
    type = ADHDGDiffusion
    variable = vel_x
    alpha = 6
    diff = 'mu'
    side_variable = vel_bar_x
  []
  [momentum_x_diffusion_side]
    type = ADHDGDiffusionSide
    variable = vel_bar_x
    alpha = 6
    diff = 'mu'
    interior_variable = vel_x
  []
  [momentum_x_pressure]
    type = ADHDGPressure
    variable = vel_x
    pressure = pressure_bar
    component = 0
  []

  [momentum_y_convection]
    type = ADHDGAdvection
    variable = vel_y
    velocity = 'velocity'
    coeff = ${rho}
    side_variable = vel_bar_y
  []
  [momentum_y_convection_side]
    type = ADHDGAdvectionSide
    variable = vel_bar_y
    velocity = 'velocity'
    coeff = ${rho}
    interior_variable = vel_y
  []
  [momentum_y_diffusion]
    type = ADHDGDiffusion
    variable = vel_y
    alpha = 6
    diff = 'mu'
    side_variable = vel_bar_y
  []
  [momentum_y_diffusion_side]
    type = ADHDGDiffusionSide
    variable = vel_bar_y
    alpha = 6
    diff = 'mu'
    interior_variable = vel_y
  []
  [momentum_y_pressure]
    type = ADHDGPressure
    variable = vel_y
    pressure = pressure_bar
    component = 1
  []

  [mass_convection]
    type = ADHDGAdvection
    variable = pressure
    velocity = 'velocity'
    coeff = ${fparse -rho}
    self_advection = false
  []
  [mass_convection_bar]
    type = ADHDGAdvection
    variable = pressure_bar
    velocity = 'velocity'
    coeff = ${rho}
    self_advection = false
  []

  [pb_mass]
    type = MassMatrixDGKernel
    variable = pressure_bar
    matrix_tags = 'mass'
    density = ${fparse -gamma * omega_m}
  []

  [u_jump]
    type = MassFluxPenalty
    variable = vel_x
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
  []
  [v_jump]
    type = MassFluxPenalty
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
    gamma = ${gamma}
  []
[]

[BCs]
  [momentum_x_diffusion_walls]
    type = HDGDiffusionBC
    boundary = 'left bottom right'
    variable = vel_x
    alpha = 6
    exact_soln = '0'
    diff = 'mu'
  []
  [momentum_x_diffusion_side_walls]
    type = ADHDGSideDirichletBC
    variable = vel_bar_x
    exact_soln = 0
    boundary = 'left bottom right'
  []
  [momentum_x_diffusion_top]
    type = HDGDiffusionBC
    boundary = 'top'
    variable = vel_x
    alpha = 6
    exact_soln = '${U}'
    diff = 'mu'
  []
  [momentum_x_diffusion_side_top]
    type = ADHDGSideDirichletBC
    variable = vel_bar_x
    exact_soln = ${U}
    boundary = 'top'
  []
  [momentum_x_pressure_all]
    type = ADHDGPressureBC
    variable = vel_x
    component = 0
    pressure = pressure_bar
    boundary = 'left bottom right top'
  []

  [momentum_y_diffusion_all]
    type = HDGDiffusionBC
    boundary = 'left bottom right top'
    variable = vel_y
    alpha = 6
    exact_soln = '0'
    diff = 'mu'
  []
  [momentum_y_side_all]
    type = ADHDGSideDirichletBC
    variable = vel_bar_y
    exact_soln = 0
    boundary = 'left bottom right top'
  []
  [momentum_y_pressure_all]
    type = ADHDGPressureBC
    variable = vel_y
    component = 1
    pressure = pressure_bar
    boundary = 'left bottom right top'
  []

  [mass_convection_all]
    type = ADHDGAdvectionDirichletBC
    variable = pressure
    velocity = 'velocity'
    coeff = ${fparse -rho}
    self_advection = false
    boundary = 'left bottom top right'
  []
  [mass_convection_bar_all]
    type = ADHDGAdvectionDirichletBC
    variable = pressure_bar
    velocity = 'velocity'
    coeff = ${rho}
    self_advection = false
    boundary = 'left bottom top right'
  []
  [mass_convection_bar_diri_walls]
    type = ADHDGAdvectionDirichletBC
    variable = pressure_bar
    velocity_function = wall_vel_func
    coeff = ${fparse -rho}
    self_advection = false
    boundary = 'left bottom right'
  []
  [mass_convection_bar_diri_top]
    type = ADHDGAdvectionDirichletBC
    variable = pressure_bar
    velocity_function = top_vel_func
    coeff = ${fparse -rho}
    self_advection = false
    boundary = 'top'
  []

  [pb_mass]
    type = MassMatrixIntegratedBC
    variable = pressure_bar
    matrix_tags = 'mass'
    boundary = 'left right bottom top'
    density = ${fparse -gamma * omega_m}
  []

  [u_jump]
    type = MassFluxPenaltyBC
    variable = vel_x
    u = vel_x
    v = vel_y
    component = 0
    boundary = 'left right bottom top'
    gamma = ${gamma}
  []
  [v_jump]
    type = MassFluxPenaltyBC
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
    boundary = 'left right bottom top'
    gamma = ${gamma}
  []
[]

[Functions]
  [top_vel_func]
    type = ParsedVectorFunction
    expression_x = ${U}
  []
  [wall_vel_func]
    type = ParsedVectorFunction
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [vel]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = 'velocity'
    u = vel_x
    v = vel_y
  []
  [rhou]
    type = ADParsedMaterial
    property_name = 'rhou'
    coupled_variables = 'vel_x'
    material_property_names = 'rho'
    expression = 'rho*vel_x'
  []
  [rhov]
    type = ADParsedMaterial
    property_name = 'rhov'
    coupled_variables = 'vel_y'
    material_property_names = 'rho'
    expression = 'rho*vel_y'
  []
[]

[Preconditioning]
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
        vars = 'vel_x vel_y vel_bar_x vel_bar_y'
        # petsc_options = '-ksp_converged_reason'
        petsc_options_iname = '-pc_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side'
        petsc_options_value = 'lu       gmres     1e-2      300                right'
      []
      [p]
        vars = 'pressure pressure_bar'
        petsc_options = '-ksp_converged_reason'
        petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side'
        petsc_options_value = 'gmres     300                1e-2      lu       right'
      []
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'pressure_integral'
  []
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    pp_names = ''
    function = '${rho} * ${U} * ${l} / ${mu}'
  []
  [symmetric]
    type = IsMatrixSymmetric
  []
  [pressure_integral]
    type = ElementIntegralVariablePostprocessor
    variable = pressure
    execute_on = linear
  []
[]
