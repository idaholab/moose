[Mesh]
  [drmg]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 30
    ny = 30
    elem_type = QUAD4
    partition = square
  []
[]

[Variables]
  [u][]
[]

[AuxVariables]
  [source][]
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [flux]
    type = CoupledVarNeumannBC
    variable = u
    boundary = 'right'
    v = source
  []
  [bdr]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-6
[]

[Outputs]
  exodus = true
[]
