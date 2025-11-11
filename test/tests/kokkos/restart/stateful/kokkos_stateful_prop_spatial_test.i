[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [prop1]
    order = SECOND
    family = MONOMIAL
  []
[]

[AuxKernels]
  [prop1_output]
    type = KokkosMaterialRealAux
    variable = prop1
    property = thermal_conductivity
  []
[]

[Kernels]
  [heat]
    type = KokkosMatDiffusionTest
    variable = u
    prop_name = thermal_conductivity
  []
  [ie]
    type = KokkosTimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 3
    value = 0.0
  []
  [right]
    type = KokkosMTBC
    variable = u
    boundary = 1
    grad = 1.0
    prop_name = thermal_conductivity
  []
[]

[Materials]
  [stateful]
    type = KokkosStatefulSpatialTest
    block = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  start_time = 0.0
  num_steps = 3
  dt = .1
[]

[Outputs]
  checkpoint = true
[]
