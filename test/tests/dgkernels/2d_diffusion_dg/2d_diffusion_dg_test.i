###########################################################
# This is a test of the Discontinuous Galerkin System.
# Discontinous basis functions are used (Monomials) and
# a Laplacian DGKernel contributes to the
# internal edges around each element. Jumps are allowed
# but penalized by this method.
#
# @Requirement F3.60
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
#  xmin = -1
#  xmax = 1
#  ymin = -1
#  ymax = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = MONOMIAL

    [./InitialCondition]
      type = ConstantIC
      value = 1
    [../]
  [../]
[]

[Functions]
  active = 'forcing_fn exact_fn'

  [./forcing_fn]
    type = ParsedFunction
#    function = -4.0+(x*x)+(y*y)
#    function = x
#    function = (x*x)-2.0
    expression = 2*pow(e,-x-(y*y))*(1-2*y*y)
#    function = (x*x*x)-6.0*x
  [../]

  [./exact_fn]
    type = ParsedGradFunction
#    function = x
#    grad_x = 1
#    grad_y = 0

#    function = (x*x)+(y*y)
#    grad_x = 2*x
#    grad_y = 2*y

#    function = (x*x)
#    grad_x = 2*x
#    grad_y = 0

    value = pow(e,-x-(y*y))
    grad_x = -pow(e,-x-(y*y))
    grad_y = -2*y*pow(e,-x-(y*y))

#    function = (x*x*x)
#    grad_x = 3*x*x
#    grad_y = 0
  [../]
[]

[Kernels]
  active = 'diff abs forcing'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./abs]          # u * v
    type = Reaction
    variable = u
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[DGKernels]
  active = 'dg_diff'

  [./dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  [../]
[]

[BCs]
  active = 'all'

  [./all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
    epsilon = -1
    sigma = 6
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
#  petsc_options = '-snes_mf'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre    boomeramg'

#  petsc_options = '-snes_mf'
#  max_r_steps = 2
  [./Adaptivity]
    steps = 2
    refine_fraction = 1.0
    coarsen_fraction = 0
    max_h_level = 8
  [../]

  nl_rel_tol = 1e-10

#  nl_rel_tol = 1e-12
[]

[Postprocessors]
  active = 'h dofs l2_err'

  [./h]
    type = AverageElementSize
  [../]

  [./dofs]
    type = NumDOFs
  [../]

  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Outputs]
  file_base = out
  exodus = true
[]
