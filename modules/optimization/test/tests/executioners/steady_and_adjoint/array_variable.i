[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymax = 1
    nx = 10
    ny = 10
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
[]

[Variables]
  [u]
    components = 2
  []
  [u_adjoint]
    components = 2
    nl_sys = adjoint
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = D
  []
  [reaction]
    type = ArrayReaction
    variable = u
    reaction_coefficient = A
  []
  [src_u]
    type = ArrayBodyForce
    variable = u
    function = '1 0'
  []
  [src_adjoint]
    type = ArrayBodyForce
    variable = u_adjoint
    function = '0 1'
  []
[]

[Materials]
  [diffc]
    type = GenericConstantArray
    prop_name = 'D'
    prop_value = '1 1'
  []
  [coeff]
    type = GenericConstant2DArray
    prop_name = 'A'
    prop_value = '0 -10; -1 0'
  []
[]

[BCs]
  [dirichlet]
    type = ArrayDirichletBC
    variable = u
    boundary = 'top right'
    values = '0 0'
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint

  nl_rel_tol = 1e-12
  l_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
