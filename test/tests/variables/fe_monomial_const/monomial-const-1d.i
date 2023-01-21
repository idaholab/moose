[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 100
  elem_type = EDGE3
[]

[Functions]
  [./bc_fn]
    type=ParsedFunction
    expression=0
  [../]

  [./forcing_fn]
    type = MTPiecewiseConst1D
  [../]

  [./solution]
    type = MTPiecewiseConst1D
  [../]
[]

[Variables]
  [./u]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  active = 'diff forcing reaction'
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./reaction]
    type = Reaction
    variable = u
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  # Note: MOOSE's DirichletBCs do not work properly with shape functions that do not
  #       have DOFs at the element edges.  This test works because the solution
  #       has been designed to be zero at the boundary which is satisfied by the IC
  #       Ticket #1352
  active = ''
  [./bc_all]
    type=FunctionDirichletBC
    variable = u
    boundary = 'left right'
    function = bc_fn
  [../]
[]

[Postprocessors]
  [./dofs]
    type = NumDOFs
  [../]

  [./h]
    type = AverageElementSize
  [../]

  [./L2error]
    type = ElementL2Error
    variable = u
    function = solution
  [../]
  [./H1error]
    type = ElementH1Error
    variable = u
    function = solution
  [../]
  [./H1Semierror]
    type = ElementH1SemiError
    variable = u
    function = solution
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
  nl_rel_tol = 1.e-9
  [./Adaptivity]

  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
