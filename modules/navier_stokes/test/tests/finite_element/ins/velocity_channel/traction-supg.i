# This input file tests outflow boundary conditions for the incompressible NS equations.

[GlobalParams]
  gravity = '0 0 0'
  integrate_p_by_parts = true
  supg = true
  preset = false
  laplace = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 3.0
  ymin = 0
  ymax = 1.0
  nx = 30
  ny = 10
  elem_type = QUAD9
[]


[Variables]
  [vel_x]
    order = SECOND
    family = LAGRANGE
  []
  [vel_y]
    order = SECOND
    family = LAGRANGE
  []
  [p]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
  []
  [x_momentum_space]
    type = INSMomentumTractionForm
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
  []
  [y_momentum_space]
    type = INSMomentumTractionForm
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
  []
[]

[BCs]
  [x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'top bottom'
    value = 0.0
  []
  [y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'left top bottom'
    value = 0.0
  []
  [x_inlet]
    type = FunctionDirichletBC
    variable = vel_x
    boundary = 'left'
    function = 'inlet_func'
  []
[]

[Materials]
  [const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1  1'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    solve_type = NEWTON
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = none
  nl_rel_tol = 1e-12
[]

[Outputs]
  [out]
    type = Exodus
  []
[]

[Functions]
  [inlet_func]
    type = ParsedFunction
    expression = '-4 * (y - 0.5)^2 + 1'
  []
[]
