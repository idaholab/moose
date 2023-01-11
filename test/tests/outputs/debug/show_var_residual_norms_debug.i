[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD9
[]

[Functions]
  [./forcing_fnu]
    type = ParsedFunction
    expression = -5.8*(x+y)+x*x*x-x+y*y*y-y
  [../]
  [./forcing_fnv]
    type = ParsedFunction
    expression = -4
  [../]

  [./slnu]
    type = ParsedGradFunction
    value = x*x*x-x+y*y*y-y
    grad_x = 3*x*x-1
    grad_y = 3*y*y-1
  [../]
  [./slnv]
    type = ParsedGradFunction
    value = x*x+y*y
    grad_x = 2*x
    grad_y = 2*y
  [../]

  #NeumannBC functions
  [./bc_fnut]
    type = ParsedFunction
    expression = 3*y*y-1
  [../]
  [./bc_fnub]
    type = ParsedFunction
    expression = -3*y*y+1
  [../]
  [./bc_fnul]
    type = ParsedFunction
    expression = -3*x*x+1
  [../]
  [./bc_fnur]
    type = ParsedFunction
    expression = 3*x*x-1
  [../]
[]

[Variables]
  [./u]
    order = THIRD
    family = HIERARCHIC
  [../]
  [./v]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff1 diff2 test1 forceu forcev react'
  [./diff1]
    type = Diffusion
    variable = u
  [../]
  [./test1]
    type = CoupledConvection
    variable = u
    velocity_vector = v
  [../]
  [./diff2]
    type = Diffusion
    variable = v
  [../]
  [./react]
    type = Reaction
    variable = u
  [../]

  [./forceu]
    type = BodyForce
    variable = u
    function = forcing_fnu
  [../]
  [./forcev]
    type = BodyForce
    variable = v
    function = forcing_fnv
  [../]

[]

[BCs]
  active = 'bc_u_tb bc_v bc_ul bc_ur bc_ut bc_ub'
  [./bc_u]
    type = FunctionPenaltyDirichletBC
    variable = u
    function = slnu
    boundary = 'left right top bottom'
    penalty = 1e6
  [../]
  [./bc_v]
    type = FunctionDirichletBC
    variable = v
    function = slnv
    boundary = 'left right top bottom'
  [../]

  [./bc_u_lr]
    type = FunctionPenaltyDirichletBC
    variable = u
    function = slnu
    boundary = 'left right top bottom'
    penalty = 1e6
  [../]
  [./bc_u_tb]
    type = CoupledKernelGradBC
    variable = u
    var2 = v
    vel = '0.1 0.1'
    boundary = 'top bottom left right'
  [../]

  [./bc_ul]
    type = FunctionNeumannBC
    variable = u
    function = bc_fnul
    boundary = 'left'
  [../]
  [./bc_ur]
    type = FunctionNeumannBC
    variable = u
    function = bc_fnur
    boundary = 'right'
  [../]
  [./bc_ut]
    type = FunctionNeumannBC
    variable = u
    function = bc_fnut
    boundary = 'top'
  [../]
  [./bc_ub]
    type = FunctionNeumannBC
    variable = u
    function = bc_fnub
    boundary = 'bottom'
  [../]
[]

[Preconditioning]
  active = ' '
  [./prec]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  active='L2u L2v'
  [./dofs]
    type = NumDOFs
  [../]

  [./h]
    type = AverageElementSize
  [../]

  [./L2u]
    type = ElementL2Error
    variable = u
    function = slnu
  [../]
  [./L2v]
    type = ElementL2Error
    variable = v
    function = slnv
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
#  petsc_options = '-snes'
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-13
[]

[Outputs]
  execute_on = 'timestep_end'
[]

[Debug]
  show_var_residual_norms = true
[]
