#
# Compare a diffusion equation with (c) and without (v) periodic gradient
# constraints and a ramped sloped initial condition and value-periodic diffusion (p)
# without a slope.
#

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 40
    ny = 40
  []
  [secondary_x]
    input = gen
    type = LowerDBlockFromSidesetGenerator
    sidesets = '3'
    new_block_id = 10
    new_block_name = "secondary_x"
  []
  [primary_x]
    input = secondary_x
    type = LowerDBlockFromSidesetGenerator
    sidesets = '1'
    new_block_id = 12
    new_block_name = "primary_x"
  []
  [secondary_y]
    input = primary_x
    type = LowerDBlockFromSidesetGenerator
    sidesets = '0'
    new_block_id = 11
    new_block_name = "secondary_y"
  []
  [primary_y]
    input = secondary_y
    type = LowerDBlockFromSidesetGenerator
    sidesets = '2'
    new_block_id = 13
    new_block_name = "primary_y"
  []
[]

[Functions]
  [./init_slope]
    # slope with a concentration spike close to the lower interface
    type = ParsedFunction
    expression = 'if(x>0.4 & x<0.6 & y>0.1 & y<0.3, 3+y, y)'
  [../]
  [./init_flat]
    # no-slope and the same spike
    type = ParsedFunction
    expression = 'if(x>0.4 & x<0.6 & y>0.1 & y<0.3, 3, 0)'
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
    block = "secondary_x"
  [../]
  [./lm_left_right_y]
    order = FIRST
    family = LAGRANGE
    block = "secondary_x"
  [../]

  # Lagrange multipliers for gradient component in the vertical directon
  [./lm_up_down_x]
    order = FIRST
    family = LAGRANGE
    block = "secondary_y"
  [../]
  [./lm_up_down_y]
    order = FIRST
    family = LAGRANGE
    block = "secondary_y"
  [../]
[]

[Kernels]
  # the gradient constrained concentration
  [./diff]
    type = Diffusion
    variable = c
    block = 0
  [../]
  [./dt]
    type = TimeDerivative
    variable = c
    block = 0
  [../]

  # the un-constrained concentration
  [./diff2]
    type = Diffusion
    variable = v
    block = 0
  [../]
  [./dt2]
    type = TimeDerivative
    variable = v
    block = 0
  [../]

  # the value periodic concentration
  [./diff3]
    type = Diffusion
    variable = p
    block = 0
  [../]
  [./dt3]
    type = TimeDerivative
    variable = p
    block = 0
  [../]
[]

[Constraints]
  [./equaly_grad_x]
    type = EqualGradientConstraint
    variable = lm_up_down_x
    component = 0
    secondary_variable = c
    secondary_boundary = bottom
    primary_boundary = top
    secondary_subdomain = secondary_y
    primary_subdomain = primary_y
    periodic = true
  [../]
  [./equaly_grad_y]
    type = EqualGradientConstraint
    variable = lm_up_down_y
    component = 1
    secondary_variable = c
    secondary_boundary = bottom
    primary_boundary = top
    secondary_subdomain = secondary_y
    primary_subdomain = primary_y
    periodic = true
  [../]

  [./equalx_grad_x]
    type = EqualGradientConstraint
    variable = lm_left_right_x
    component = 0
    secondary_variable = c
    secondary_boundary = left
    primary_boundary = right
    secondary_subdomain = secondary_x
    primary_subdomain = primary_x
    periodic = true
  [../]
  [./equalx_grad_y]
    type = EqualGradientConstraint
    variable = lm_left_right_y
    component = 1
    secondary_variable = c
    secondary_boundary = left
    primary_boundary = right
    secondary_subdomain = secondary_x
    primary_subdomain = primary_x
    periodic = true
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
    expression = 'c-v'
    coupled_variables = 'c v'
    block = 0
  [../]

  # difference between the periodic gradient constrained diffusion and the flat
  # value period diffusien with a constant slope added. This should be the same,
  # but they aren't quite because the gradient constraint affects the gradient in
  # the entire elements (i.e. a larger volume is affected by the gradient constraint
  # compared to the nodal value periodicity)
  [./diff_periodic]
    type = ParsedAux
    variable = diff_periodic
    expression = 'c-p-slope'
    coupled_variables = 'c p slope'
    block = 0
  [../]

  # subtract the constant slope from the gradient periodic simulation (should yield
  # almost p - per the argument above)
  [./diff_slope]
    type = ParsedAux
    variable = diff_slope
    expression = 'c-slope'
    coupled_variables = 'c slope'
    block = 0
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
  dt = 0.01
  num_steps = 20
[]

[Outputs]
  exodus = true
[]
