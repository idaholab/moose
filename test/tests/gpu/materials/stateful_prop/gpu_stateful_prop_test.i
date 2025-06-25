[Mesh]
  dim = 3
  file = cube.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

# [AuxVariables]
#   [./prop1]
#     order = CONSTANT
#     family = MONOMIAL
#   [../]
# []

[KokkosKernels]
  [./heat]
    type = KokkosMatDiffusionTest
    variable = u
    prop_name = thermal_conductivity
    prop_state = 'older'                  # Use the "Older" value to compute conductivity
  [../]

  [./ie]
    type = KokkosTimeDerivative
    variable = u
  [../]
[]

# [AuxKernels]
#   [./prop1_output_init]
#     type = MaterialRealAux
#     variable = prop1
#     property = thermal_conductivity
#     execute_on = initial
#   [../]
#
#   [./prop1_output]
#     type = MaterialRealAux
#     variable = prop1
#     property = thermal_conductivity
#   [../]
# []

[KokkosBCs]
  [./bottom]
    type = KokkosDirichletBC
    variable = u
    boundary = 1
    value = 0.0
  [../]

  [./top]
    type = KokkosDirichletBC
    variable = u
    boundary = 2
    value = 1.0
  [../]
[]

[KokkosMaterials]
  [./stateful]
    type = KokkosStatefulTest
    prop_names = thermal_conductivity
    prop_values = 1.0
  [../]
[]

# [Postprocessors]
#   [./integral]
#     type = ElementAverageValue
#     variable = prop1
#     execute_on = 'initial timestep_end'
#   [../]
# []

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-vec_type -nl0_mat_type -pc_type -pc_hypre_type'
  petsc_options_value = 'kokkos hypre hypre boomeramg'

  start_time = 0.0
  num_steps = 5
  dt = .1
[]

[Outputs]
  file_base = out_older_gpu
  exodus = true
  csv = true
[]
