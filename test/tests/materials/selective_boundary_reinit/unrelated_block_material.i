[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[AuxVariables]
  [needed]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [needed]
    type = MaterialRealAux
    property = needed_prop
    variable = needed
    execute_on = 'TIMESTEP_END'
  []
[]

[Materials]
  [needed_mat]
    type = BoundaryMaterialReinitTest
    property = needed_prop
    value = 2
  []
  [unrelated_mat]
    type = BoundaryMaterialReinitTest
    property = unrelated_prop
    value = 3
    error_on_volume = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1
[]
