#
# Compare a diffusion equation with (c) and without (v) periodic gradient
# constraints and a ramped sloped initial condition and value-periodic diffusion (p)
# without a slope.
#

[Mesh]
  type = MortarPeriodicMesh
  dim = 2
  nx = 40
  ny = 40
  periodic_directions = 'x y'

  [./MortarInterfaces]
    [./left_right]
      master = 1
      slave = 3
      subdomain = 10
    [../]
    [./up_down]
      master = 0
      slave = 2
      subdomain = 11
    [../]
  [../]
[]

[Functions]
  [./init_slope]
    # slope with a concentration spike close to the lower interface
    type = ParsedFunction
    value = 'if(x>0.4 & x<0.6 & y>0.1 & y<0.3, 3+y, y)'
  [../]
  [./init_flat]
    # no-slope and the same spike
    type = ParsedFunction
    value = 'if(x>0.4 & x<0.6 & y>0.1 & y<0.3, 3, 0)'
  [../]
[]

[Variables]
  # gradient constrained concentration
  [./c]
    order = FIRST
    family = LAGRANGE
    block = 0
    [./InitialCondition]
      type = FunctionIC
      function = init_slope
    [../]
  [../]

  # unconstrained concentrarion
  [./v]
    order = FIRST
    family = LAGRANGE
    block = 0
    [./InitialCondition]
      type = FunctionIC
      function = init_slope
    [../]
  [../]

  # flat value periodic diffusion
  [./p]
    order = FIRST
    family = LAGRANGE
    block = 0
    [./InitialCondition]
      type = FunctionIC
      function = init_flat
    [../]
  [../]

  # Lagrange multipliers for gradient component in the horizontal directon
  [./lm_left_right_x]
    order = FIRST
    family = LAGRANGE
    block = 10
  [../]
  [./lm_left_right_y]
    order = FIRST
    family = LAGRANGE
    block = 10
  [../]

  # Lagrange multipliers for gradient component in the vertical directon
  [./lm_up_down_x]
    order = FIRST
    family = LAGRANGE
    block = 11
  [../]
  [./lm_up_down_y]
    order = FIRST
    family = LAGRANGE
    block = 11
  [../]
[]

[Kernels]
  # the gradient constrained concentration
  [./diff]
    type = Diffusion
    variable = c
  [../]
  [./dt]
    type = TimeDerivative
    variable = c
  [../]

  # the un-constrained concentration
  [./diff2]
    type = Diffusion
    variable = v
  [../]
  [./dt2]
    type = TimeDerivative
    variable = v
  [../]

  # the value periodic concentration
  [./diff3]
    type = Diffusion
    variable = p
  [../]
  [./dt3]
    type = TimeDerivative
    variable = p
  [../]
[]

[Constraints]
  [./equaly_grad_x]
    type = EqualGradientConstraint
    variable = lm_up_down_x
    interface = up_down
    component = 0
    master_variable = c
  [../]
  [./equaly_grad_y]
    type = EqualGradientConstraint
    variable = lm_up_down_y
    interface = up_down
    component = 1
    master_variable = c
  [../]

  [./equalx_grad_x]
    type = EqualGradientConstraint
    variable = lm_left_right_x
    interface = left_right
    component = 0
    master_variable = c
  [../]
  [./equalx_grad_y]
    type = EqualGradientConstraint
    variable = lm_left_right_y
    interface = left_right
    component = 1
    master_variable = c
  [../]
[]

[BCs]
  # DiffusionFluxBC is the surface term in the weak form of the Diffusion equation
  [./surface]
    type = DiffusionFluxBC
    boundary = 'top bottom left right'
    variable = c
  [../]
  [./surface2]
    type = DiffusionFluxBC
    boundary = 'top bottom left right'
    variable = v
  [../]

  # for the value periodic diffusion we skip the surface term and apply value PBCs
  [./Periodic]
    [./up_down]
      variable = p
      primary = 0
      secondary = 2
      translation = '0 1 0'
    [../]
    [./left_right]
      variable = p
      primary = 1
      secondary = 3
      translation = '-1 0 0'
    [../]
  [../]
[]

[AuxVariables]
  [./diff_constraint]
    block = 0
  [../]
  [./diff_periodic]
    block = 0
  [../]
  [./diff_slope]
    block = 0
  [../]
  [./slope]
    block = 0
    [./InitialCondition]
      type = FunctionIC
      function = y
    [../]
  [../]
[]

[AuxKernels]
  # difference between the constrained and unconstrained sloped diffusions
  [./diff_constraint]
    type = ParsedAux
    variable = diff_constraint
    function = 'c-v'
    args = 'c v'
  [../]

  # difference between the periodic gradient constrained diffusion and the flat
  # value period diffusien with a constant slope added. This should be the same,
  # but they aren't quite because the gradient constraint affects the gradient in
  # the entire elements (i.e. a larger volume is affected by the gradient constraint
  # compared to teh nodal value periodicity)
  [./diff_periodic]
    type = ParsedAux
    variable = diff_periodic
    function = 'c-p-slope'
    args = 'c p slope'
  [../]

  # subtract the constant slope from the gradient periodic simulation (should yield
  # almost p - per the argument above)
  [./diff_slope]
    type = ParsedAux
    variable = diff_slope
    function = 'c-slope'
    args = 'c slope'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  # the shift is necessary to facilitate the solve. The Lagrange multipliers
  # introduce zero-on diaginal blocks, which make the matrix hard to invert.
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       NONZERO               1e-10'

  nl_rel_tol = 1e-11
  nl_abs_tol = 1e-10
  l_tol = 1e-10
  l_abs_step_tol = 1e-11
  dt = 0.01
  num_steps = 20
[]

[Outputs]
  exodus = true
[]
