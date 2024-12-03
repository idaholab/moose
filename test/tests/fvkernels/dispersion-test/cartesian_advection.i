[GlobalParams]
  advected_interp_method = 'min_mod' #average upwind sou min_mod vanLeer quick venkatakrishnan skewness-corrected
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 11
    ny = 11
  []
[]

[Variables]
  [scalar]
    type = MooseVariableFVReal
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  [time_derivative]
    type = FVTimeKernel
    variable = scalar
  []
  [scalar_advection]
    type = FVAdvection
    variable = scalar
    velocity = '1 1 0'
  []
[]

[FVBCs]
  [inflow_1]
    type = FVDirichletBC
    boundary = 'left'
    value = '1'
    variable = scalar
  []
  [inflow_0]
    type = FVDirichletBC
    boundary = 'bottom'
    value = '0'
    variable = scalar
  []
  [outflow]
    type = FVConstantScalarOutflowBC
    variable = scalar
    velocity = '1 1 0'
    boundary = 'right top'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  dt = 0.1
  end_time = 10.0
  steady_state_detection = true
  steady_state_tolerance = 1e-12
  nl_abs_tol = 1e-12
  line_search = none
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'final'
  []
[]
