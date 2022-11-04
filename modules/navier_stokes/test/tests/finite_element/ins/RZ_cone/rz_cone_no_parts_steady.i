# This input file tests several different things:
# .) The axisymmetric (RZ) form of the governing equations.
# .) An open boundary.
# .) Not integrating the pressure by parts, thereby requiring a pressure pin.
# .) Natural boundary condition at the outlet.
[GlobalParams]
  integrate_p_by_parts = false
  laplace = true
  gravity = '0 0 0'
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

[Variables]
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
  [./p]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[BCs]
  [./p_corner]
    # This is required, because pressure term is *not* integrated by parts.
    type = DirichletBC
    boundary = top_right
    value = 0
    variable = p
  [../]
  [./u_in]
    type = DirichletBC
    boundary = bottom
    variable = vel_x
    value = 0
  [../]
  [./v_in]
    type = FunctionDirichletBC
    boundary = bottom
    variable = vel_y
    function = 'inlet_func'
  [../]
  [./u_axis_and_walls]
    type = DirichletBC
    boundary = 'left right'
    variable = vel_x
    value = 0
  [../]
  [./v_no_slip]
    type = DirichletBC
    boundary = 'right'
    variable = vel_y
    value = 0
  [../]
[]


[Kernels]
  [./mass]
    type = INSMassRZ
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
  [../]
  [./x_momentum_space]
    type = INSMomentumLaplaceFormRZ
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
  [../]
  [./y_momentum_space]
    type = INSMomentumLaplaceFormRZ
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 'volume'
    prop_names = 'rho mu'
    prop_values = '1  1'
  [../]
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
