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
  []
  [u_adjoint]
    nl_sys = adjoint
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [src]
    type = BodyForce
    variable = u
    value = 1
  []
  [src_adjoint]
    type = BodyForce
    variable = u_adjoint
    value = 10
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = u
    boundary = 'top right'
    value = 0
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
