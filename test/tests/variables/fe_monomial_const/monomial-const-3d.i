[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
  nx = 21
  ny = 21
  nz = 21
  elem_type = HEX8
[]

[Functions]
  [./bc_fn]
    type=ParsedFunction
    expression=0
  [../]
  [./bc_fnt]
    type = ParsedFunction
    expression = 0
  [../]
  [./bc_fnb]
    type = ParsedFunction
    expression = 0
  [../]
  [./bc_fnl]
    type = ParsedFunction
    expression = 0
  [../]
  [./bc_fnr]
    type = ParsedFunction
    expression = 0
  [../]

  [./forcing_fn]
#    type = ParsedFunction
#    expression = 0
    type = MTPiecewiseConst3D
  [../]

  [./solution]
    type = MTPiecewiseConst3D
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
    boundary = 'top bottom left right'
    function = bc_fn
  [../]
  [./bc_top]
    type = FunctionNeumannBC
    variable = u
    boundary = 'top'
    function = bc_fnt
  [../]
  [./bc_bottom]
    type = FunctionNeumannBC
    variable = u
    boundary = 'bottom'
    function = bc_fnb
  [../]
  [./bc_left]
    type = FunctionNeumannBC
    variable = u
    boundary = 'left'
    function = bc_fnl
  [../]
  [./bc_right]
    type = FunctionNeumannBC
    variable = u
    boundary = 'right'
    function = bc_fnr
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
