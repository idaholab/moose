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
  [v]
  []
  [u_adjoint]
    nl_sys = adjoint
  []
  [v_adjoint]
    nl_sys = adjoint
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [uv]
    type = CoupledForce
    variable = u
    v = v
    coef = 10
  []
  [vu]
    type = CoupledForce
    variable = v
    v = u
    coef = 1
  []
  [src_u]
    type = BodyForce
    variable = u
    value = 1
  []
  [src_u_adjoint]
    type = BodyForce
    variable = u_adjoint
    value = 0
  []
  [src_v_adjoint]
    type = BodyForce
    variable = v_adjoint
    value = 1
  []
[]

[BCs]
  [dirichlet_u]
    type = DirichletBC
    variable = u
    boundary = 'top right'
    value = 0
  []
  [dirichlet_v]
    type = DirichletBC
    variable = v
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
