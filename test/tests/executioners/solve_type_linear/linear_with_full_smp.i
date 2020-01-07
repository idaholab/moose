[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Variables]
  [u]
  []
  [v]
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

  [u_reaction]
    type = Reaction
    variable = u
  []
  [v_reaction]
    type = Reaction
    variable = v
  []

  [u_force]
    type = BodyForce
    variable = u
  []
  [v_force]
    type = CoupledForce
    variable = v
    v = u
  []
[]

[Executioner]
  type = Steady
  solve_type = LINEAR
[]

[Outputs]
  exodus = true
[]
