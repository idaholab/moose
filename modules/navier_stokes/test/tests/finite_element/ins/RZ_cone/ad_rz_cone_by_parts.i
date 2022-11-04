[Mesh]
  file = '2d_cone.msh'
[]

[Problem]
  coord_type = RZ
[]

[AuxVariables]
  [vel_x]
    order = SECOND
  []
  [vel_y]
    order = SECOND
  []
[]

[AuxKernels]
  [vel_x]
    type = VectorVariableComponentAux
    variable = vel_x
    vector_variable = velocity
    component = 'x'
  []
  [vel_y]
    type = VectorVariableComponentAux
    variable = vel_y
    vector_variable = velocity
    component = 'y'
  []
[]

[Variables]
  [./velocity]
    order = SECOND
    family = LAGRANGE_VEC
  [../]
  [./p]
  [../]
[]

[Kernels]
  [./mass]
    type = INSADMass
    variable = p
  [../]

  [./momentum_time]
    type = INSADMomentumTimeDerivative
    variable = velocity
  [../]

  [./momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
  [../]

  [./momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  [../]

  [./momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
    integrate_p_by_parts = true
  [../]
[]

[BCs]
  [inlet]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'bottom'
    function_x = 0
    function_y = 'inlet_func'
  [../]
  [wall]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'right'
    function_x = 0
    function_y = 0
  []
  [axis]
    type = ADVectorFunctionDirichletBC
    variable = velocity
    boundary = 'left'
    set_y_comp = false
    function_x = 0
  []
[]

[Functions]
  [./inlet_func]
    type = ParsedFunction
    expression = '-4 * x^2 + 1'
  [../]
[]

[Materials]
  [./const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '1  1'
  [../]
  [ins_mat]
    type = INSADMaterial
    velocity = velocity
    pressure = p
  []
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.005
  dtmin = 0.005
  num_steps = 5
  l_max_its = 100

  # Note: The Steady executioner can be used for this problem, if you
  # drop the INSMomentumTimeDerivative kernels and use the following
  # direct solver options.
  # petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -ksp_type'
  # petsc_options_value = 'lu NONZERO 1.e-10 preonly'

  # Block Jacobi works well for this problem, as does "-pc_type asm
  # -pc_asm_overlap 2", but an overlap of 1 does not work for some
  # reason?
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = 'bjacobi  ilu          4'

  nl_rel_tol = 1e-12
  nl_max_its = 6
[]

[Outputs]
  console = true
  [./out]
    type = Exodus
  [../]
[]

[Postprocessors]
  [./flow_in]
    type = VolumetricFlowRate
    vel_x = vel_x
    vel_y = vel_y
    boundary = 'bottom'
    outputs = 'console'    execute_on = 'timestep_end'
  [../]
  [./flow_out]
    type = VolumetricFlowRate
    vel_x = vel_x
    vel_y = vel_y
    boundary = 'top'
    outputs = 'console'    execute_on = 'timestep_end'
  [../]
[]
