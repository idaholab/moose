[Mesh]
  type = GeneratedMesh # Can generate simple lines, rectangles and rectangular prisms
  dim = 2 # Dimension of the mesh
  nx = 10 # Number of elements in the x direction
  ny = 10 # Number of elements in the y direction
  xmax = 1.0
  ymax = 1.0
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    two_term_boundary_expansion = false
  []
[]

[AuxVariables]
  [flux_x]
    type = MooseVariableFVReal
    order = CONSTANT
    family = MONOMIAL
  []
[]


[ICs]
  [u_ic]
    type = FunctionIC
    variable = u
    function = 'r2 := (x - 0.5)*(x - 0.5) + (y - 0.3)*(y - 0.3); exp(-r2 * 20)'
  []
[]

[FVKernels]
  [advection]
    type = FVAdvection
    variable = u
    velocity = '1 0.5 0'
  []
  [time]
    type = FVTimeKernel
    variable = u
  []
[]


[FVBCs]
  [fv_outflow]
    type = FVConstantScalarOutflowBC
    velocity = '1 0.5 0'
    variable = u
    boundary = 'right top'
  []
[]

[AuxKernels]
  [flux_x]
    type = AdvectiveFluxAux
    variable = flux_x
    vel_x = 1
    vel_y = 0.5
    advected_variable = u
    component = normal
    boundary = 'left right'
    check_boundary_restricted = false
  []
[]

[Postprocessors]
  [flux_right]
    type = SideIntegralVariablePostprocessor
    variable = flux_x
    boundary = 'right'
  []
  [flux_right_exact]
    type = SideAdvectiveFluxIntegral
    vel_x = 1
    vel_y = 0.5
    component = normal
    advected_quantity = u
    boundary = 'right'
  []
  [flux_left]
    type = SideIntegralVariablePostprocessor
    variable = flux_x
    boundary = 'left'
  []
  [flux_left_exact]
    type = SideAdvectiveFluxIntegral
    vel_x = 1
    vel_y = 0.5
    component = normal
    advected_quantity = u
    boundary = 'left'
  []
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_converged_reason'
  num_steps = 10
  dt = 0.25
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
