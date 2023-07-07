[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
  elem_type = HEX8
[]

[Variables]
  [u]
    order = FIRST
    family = MONOMIAL
    [InitialCondition]
      type = ConstantIC
      value = 0.5
    []
  []
[]

[Functions]
  [forcing_fn]
    type = ParsedFunction
    expression = 2*pow(e,-x-(y*y))*(1-2*y*y)
  []
  [exact_fn]
    type = ParsedGradFunction
    expression = pow(e,-x-(y*y))
    grad_x = -pow(e,-x-(y*y))
    grad_y = -2*y*pow(e,-x-(y*y))
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [abs]
    type = Reaction
    variable = u
  []
  [forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  []
[]

[DGKernels]
  [dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  []
[]

[BCs]
  [all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3 4 5'
    function = exact_fn
    epsilon = -1
    sigma = 6
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  [Adaptivity]
    switch_h_to_p_refinement = true
    steps = 2
    refine_fraction = 1.0
    coarsen_fraction = 0
    max_h_level = 8
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = 'initial timestep_end'
  []
  [dofs]
    type = NumDOFs
    execute_on = 'initial timestep_end'
  []
  [l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  exodus = true
[]
