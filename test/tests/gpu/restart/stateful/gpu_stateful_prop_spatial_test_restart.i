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
  [./u]
  [../]
[]

# [AuxVariables]
#   [./prop1]
#     order = SECOND
#     family = MONOMIAL
#   [../]
# []

# [AuxKernels]
#   [./prop1_output]
#     type = MaterialRealAux
#     variable = prop1
#     property = thermal_conductivity
#   [../]
# []

[KokkosKernels]
  [./heat]
    type = KokkosMatDiffusionTest
    variable = u
    prop_name = thermal_conductivity
  [../]
  [./ie]
    type = KokkosTimeDerivative
    variable = u
  [../]
[]

[KokkosBCs]
  [./left]
    type = KokkosDirichletBC
    variable = u
    boundary = 3
    value = 0.0
  [../]
  [./right]
    type = KokkosMTBC
    variable = u
    boundary = 1
    grad = 1.0
    prop_name = thermal_conductivity
  [../]
[]

[KokkosMaterials]
  active = stateful
  [./stateful]
    type = KokkosStatefulSpatialTest
    block = 0
  [../]
  [./another_stateful]
    type = KokkosStatefulSpatialTest
    block = 0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  start_time = 0.3
  num_steps = 2
  dt = .1
[]

[Outputs]
  file_base = out_spatial_restart_gpu
  [./out]
    type = Exodus
    elemental_as_nodal = true
    execute_elemental_on = none
  [../]
[]

[Problem]
  restart_file_base = out_spatial_gpu_cp/LATEST
[]
