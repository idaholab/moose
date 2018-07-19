[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  elem_type = QUAD4
  uniform_refine = 3
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [u_x_recovered]
  []
  [u_y_recovered]
  []
  [u_x_monomial]
    order = FIRST
    family = MONOMIAL
  []
  [u_y_monomial]
    order = FIRST
    family = MONOMIAL
  []
[]

[AuxKernels]
  [u_x_patch]
    type = NodalGradientAux
    variable = u_x_recovered
    coupled_var = u
    order = 2
    component = 0
    execute_on = 'timestep_end'
  []
  [u_y_patch]
    type = NodalGradientAux
    variable = u_y_recovered
    coupled_var = u
    order = 2
    component = 1
    execute_on = 'timestep_end'
  []
  [u_x_L2]
    type = VariableGradientComponent
    variable = u_x_monomial
    gradient_variable = u
    component = x
    execute_on = 'timestep_end'
  []
  [u_y_L2]
    type = VariableGradientComponent
    variable = u_y_monomial
    gradient_variable = u
    component = y
    execute_on = 'timestep_end'
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
  [body_force]
    type = BodyForce
    variable = u
    function = -2*y+54*x*y-12*x*y^2+16*y^2-14*y^3-4*x^3-12*x+16*x^2-42*x^2*y
  []
[]

[BCs]
  [xfix]
    type = FunctionPresetBC
    variable = u
    boundary = 'top bottom left right'
    function = 0
  []
  [yfix]
    type = FunctionPresetBC
    variable = u
    boundary = 'top bottom left right'
    function = 0
  []
[]

[Postprocessors]
  [u_x_recovered]
    type = NodalVariableValue
    variable = u_x_recovered
    nodeid = 2
  []
  [u_y_recovered]
    type = NodalVariableValue
    variable = u_y_recovered
    nodeid = 2
  []
  [u_x_recovered_error]
    type = ElementL2Error
    variable = u_x_recovered
    function = 6*x^2*y^2-6*x^2*y+14*x*y^3-16*x*y^2+2*x*y-7*y^3+6*y^2+y
  []
  [u_y_recovered_error]
    type = ElementL2Error
    variable = u_y_recovered
    function = x+x^2-2*x^3+12*x*y-16*x^2*y+4*x^3*y-21*x*y^2+21*x^2*y^2
  []
  [u_x_monomial_error]
    type = ElementL2Error
    variable = u_x_monomial
    function = 6*x^2*y^2-6*x^2*y+14*x*y^3-16*x*y^2+2*x*y-7*y^3+6*y^2+y
  []
  [u_y_monomial_error]
    type = ElementL2Error
    variable = u_y_monomial
    function = x+x^2-2*x^3+12*x*y-16*x^2*y+4*x^3*y-21*x*y^2+21*x^2*y^2
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    ksp_norm = default
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly   lu       superlu_dist'
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  l_max_its = 100
  nl_max_its = 30
[]

[Outputs]
  interval = 1
  exodus = true
  print_linear_residuals = true
[]
