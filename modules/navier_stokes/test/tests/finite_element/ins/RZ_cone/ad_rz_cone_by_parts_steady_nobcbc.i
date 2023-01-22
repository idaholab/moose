[GlobalParams]
  integrate_p_by_parts = true
[]

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
  [outlet]
    type = INSADMomentumNoBCBC
    variable = velocity
    pressure = p
    boundary = 'top'
  []
  # When the NoBCBC is applied on the outlet boundary then there is nothing
  # constraining the pressure. Thus we must pin the pressure somewhere to ensure
  # that the problem is not singular. If the below BC is not applied then
  # -pc_type svd -pc_svd_monitor reveals a singular value
  [p_corner]
    type = DirichletBC
    boundary = top_right
    value = 0
    variable = p
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
  type = Steady

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
    execute_on = 'timestep_end'
  [../]
  [./flow_out]
    type = VolumetricFlowRate
    vel_x = vel_x
    vel_y = vel_y
    boundary = 'top'
    execute_on = 'timestep_end'
  [../]
[]
