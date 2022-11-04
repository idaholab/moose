#In order to compare the solution generated using preset BC, the penalty was set to 1e10.
#Large penalty number should be used with caution.

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = -2*(x*x+y*y-2)
  [../]

  [./solution]
    type = ParsedGradFunction
    value = (1-x*x)*(1-y*y)
    grad_x = 2*(x*y*y-x)
    grad_y = 2*(x*x*y-y)
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[NodalKernels]
  [./bc_all]
    type = PenaltyDirichletNodalKernel
    variable = u
    value = 0
    boundary = 'top left right bottom'
    penalty = 1e10
  [../]
[]

# [BCs]
#   [./fix]
#     type = DirichletBC
#     preset = true
#     variable = u
#     value = 0
#     boundary = 'top left right bottom'
#   [../]
# []

[Postprocessors]
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
  nl_rel_tol = 1e-14
[]

[Outputs]
  file_base = nodal_preset_bc_out
  exodus = true
[]
