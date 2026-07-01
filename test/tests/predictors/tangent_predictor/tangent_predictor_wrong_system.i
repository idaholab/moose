# The load tag exists, but it is associated with nl1 while the predictor is attached to nl0.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Problem]
  type = FEProblem
  nl_sys_names = 'nl0 nl1'
  extra_tag_vectors = '; load_increment'
  kernel_coverage_check = false
[]

[Variables]
  [u]
    solver_sys = nl0
  []
  [v]
    solver_sys = nl1
  []
[]

[Kernels]
  [u_diffusion]
    type = Diffusion
    variable = u
  []
  [v_diffusion]
    type = Diffusion
    variable = v
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1

  [Predictor]
    type = TangentPredictor
    scale = 1
    nl_sys = nl0
    load_vector_tag = load_increment
  []
[]
