# Adaptivity on displaced problem
# - testing initial_refinement and adaptivity as well
#
# variables:
# - u and v_aux are used for displacing the problem
# - v is used to get some refinements
#

[Mesh]
  type = GeneratedMesh
  nx = 2
  ny = 2
  dim = 2
  uniform_refine = 3
  displacements = 'u aux_v'
[]

[Functions]
  [./aux_v_fn]
    type = ParsedFunction
    expression = x*(y-0.5)/5
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'udiff uie vdiff vconv vie'

  [./udiff]
    type = Diffusion
    variable = u
  [../]

  [./uie]
    type = TimeDerivative
    variable = u
  [../]

  [./vdiff]
    type = Diffusion
    variable = v
  [../]

  [./vconv]
    type = Convection
    variable = v
    velocity = '-10 1 0'
  [../]

  [./vie]
    type = TimeDerivative
    variable = v
  [../]
[]

[BCs]
  active = 'uleft uright vleft vright'

  [./uleft]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./uright]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0.1
  [../]

  [./vleft]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 1
  [../]

  [./vright]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  [../]
[]

[AuxVariables]
  [./aux_v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./aux_k_1]
    type = FunctionAux
    variable = aux_v
    function = aux_v_fn
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 2
  dt = .1

  [./Adaptivity]
    refine_fraction = 0.2
    coarsen_fraction = 0.3
    max_h_level = 4
  [../]
[]

[Outputs]
  exodus = true
  [./displaced]
    type = Exodus
    use_displaced = true
  [../]
[]
