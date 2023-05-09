[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[AuxVariables/u]
[]

[AuxKernels]
  [u_aux]
    type = ParsedAux
    variable = u
    expression = '(x + y) * t'
    use_xyzt = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 10
[]

[Outputs]
  exodus = true
[]
