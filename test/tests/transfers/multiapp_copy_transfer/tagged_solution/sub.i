[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Problem]
  extra_tag_solutions = tagged_aux_sol
[]

[Variables/u][]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = CoupledForceLagged
    variable = u
    v = force
    tag = tagged_aux_sol
  []
[]

[BCs]
  [all]
    type = VacuumBC
    variable = u
    boundary = '0 1 2 3'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

[AuxVariables/force][]
