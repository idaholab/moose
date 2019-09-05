coef=1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Problem]
  type = ReferenceResidualProblem
  solution_variables = 'u v'
  reference_residual_variables = 'saved_u saved_v'
[]

[Variables]
  [u][]
  [v][]
[]

[AuxVariables]
  [saved_u][]
  [saved_v][]
[]

[Kernels]
  [u_diff]
    type = CoefDiffusion
    variable = u
    coef = ${coef}
    save_in = saved_u
  []
  [u_rxn]
    type = PReaction
    variable = u
    coefficient = ${coef}
    power = 2
    save_in = saved_u
  []
  [u_f]
    type = BodyForce
    variable = u
    value = ${coef}
    save_in = saved_u
  []
  [v_diff]
    type = Diffusion
    variable = v
    save_in = saved_v
  []
  [v_rxn]
    type = PReaction
    variable = v
    coefficient = 1
    power = 2
    save_in = saved_v
  []
  [v_f]
    type = BodyForce
    variable = v
    value = 1
    save_in = saved_v
  []
[]

[BCs]
  [u]
    type = RobinBC
    boundary = 'left right'
    coef = ${coef}
    variable = u
  []
  [v]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = v
  []
[]


[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
