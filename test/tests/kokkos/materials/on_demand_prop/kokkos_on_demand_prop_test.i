[Mesh]
  dim = 3
  file = cube.e
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [prop]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [diff]
    type = KokkosMatDiffusionTest
    variable = u
    prop_name = prop1
  []
[]

[AuxKernels]
  [prop1_output]
    type = KokkosMaterialRealAux
    variable = prop
    property = prop2
  []
[]

[BCs]
  [bottom]
    type = KokkosDirichletBC
    variable = u
    boundary = 1
    value = 0.0
  []
  [top]
    type = KokkosDirichletBC
    variable = u
    boundary = 2
    value = 1.0
  []
[]

[Materials]
  [on_demand]
    type = KokkosOnDemandTest
    prop_names = 'prop1 prop2 prop3 prop4'
  []
[]

[Postprocessors]
  [integral]
    type = KokkosElementAverageMaterialProperty
    mat_prop = prop3
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  csv = true
[]
