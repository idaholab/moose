[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Debug]
  show_execution_order = 'ALWAYS'
[]

[AuxVariables]
  [a]
    initial_condition = 1
  []
  [b]
    initial_condition = 2
  []
  [c]
    initial_condition = 3
  []

  [a_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 1
  []
  [b_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 2
  []
  [c_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 3
  []
  [d_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 3
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  [exact_fn]
    type = ParsedFunction
    value = t
  []

  [a_fn]
    type = ParsedFunction
    value = t
  []
  [b_fn]
    type = ParsedFunction
    value = (4-t)/2
  []
[]

[AuxKernels]
  # Nodal
  # this one needs a and b set, should run last
  [c_saux]
    type = QuotientAux
    variable = c
    numerator = a
    denominator = b
    execute_on = 'initial timestep_end'
  []
  # setting b requires a
  [b_saux]
    type = ProjectionAux
    variable = b
    v = a
    execute_on = 'linear timestep_end'
  []

  # Elements
  # this one needs a and b set, should run last
  [c_saux_elem]
    type = QuotientAux
    variable = c_elem
    numerator = a_elem
    denominator = b_elem
    execute_on = 'initial timestep_end'
  []
  # setting b requires a
  [b_saux_elem]
    type = ProjectionAux
    variable = b_elem
    v = a_elem
    execute_on = 'linear timestep_end'
  []

  # boundary auxkernel
  [real_property]
    type = MaterialRealAux
    variable = d_elem
    property = 3
    boundary = 'top bottom'
  []
[]

[Kernels]
  [ie]
    type = TimeDerivative
    variable = u
  []

  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  []
[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'

  solve_type = 'PJFNK'
  start_time = 0.0
  num_steps = 1
  dt = 1
[]
