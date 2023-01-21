[GlobalParams]
  integrate_p_by_parts = false
  viscous_form = 'traction'
[]

[Mesh]
  file = '2d_cone.msh'
[]

[Problem]
  coord_type = RZ
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = Newton
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.005
  dtmin = 0.005
  num_steps = 5
  l_max_its = 100

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

[AuxVariables]
  [./vel_x]
    # Velocity in radial (r) direction
    family = LAGRANGE
    order = SECOND
  [../]
  [./vel_y]
    # Velocity in axial (z) direction
    family = LAGRANGE
    order = SECOND
  [../]
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
  [velocity]
    family = LAGRANGE_VEC
    order = SECOND
  []
  [./p]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[BCs]
  [./p_corner]
    # This is required because of the no bcs
    type = DirichletBC
    boundary = top_right
    value = 0
    variable = p
  [../]
  [./velocity_out]
    type = INSADMomentumNoBCBC
    boundary = top
    variable = velocity
    pressure = p
  [../]
  [./velocity_in]
    type = VectorFunctionDirichletBC
    boundary = bottom
    variable = velocity
    function_x = 0
    function_y = 'inlet_func'
  [../]
  [./wall]
    type = VectorFunctionDirichletBC
    boundary = 'right'
    variable = velocity
    function_x = 0
    function_y = 0
  [../]
  [./axis]
    type = ADVectorFunctionDirichletBC
    boundary = 'left'
    variable = velocity
    set_y_comp = false
    function_x = 0
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

[Functions]
  [./inlet_func]
    type = ParsedFunction
    expression = '-4 * x^2 + 1'
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
