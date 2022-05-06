# In this test, the Master App is a 10x10 grid on the unit square, and
# there are 5 Sub Apps which correspond to each vertex of the unit square
# and the center, arranged in the following order:
# 3   4
#   2
# 0   1
# Sub Apps 0, 1, 3, and 4 currently overlap with a single element in
# each corner of the Master App, while Sub App 2 overlaps with 4
# Master App elements in the center. Note that we move the corner Sub
# Apps "outward" slightly along the diagonals to avoid ambiguity with
# which SubApp is "nearest" to a given Master App element centroid.
# This makes it easier to visually verify that the Transfers are
# working correctly.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./from_sub]
  [../]
  [./elemental_from_sub]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub]
    # Note, in case you want to modify this test.  It is important that there are
    # an odd number of apps because this way we will catch errors caused by load
    # imbalances with our -p 2 tests.
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '-0.11 -0.11 0.0
                 0.91 -0.11 0.0
                 0.4 0.4 0.0
                 -0.11 0.91 0.0
                 0.91 0.91 0.0'
    input_files = two_way_many_apps_sub.i
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./from_sub]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = u
    variable = from_sub
  [../]
  [./elemental_from_sub]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = u
    variable = elemental_from_sub
  [../]
  [./to_sub]
    type = MultiAppNearestNodeTransfer
    to_multi_app = sub
    source_variable = u
    variable = from_parent
  [../]
  [./elemental_to_sub]
    type = MultiAppNearestNodeTransfer
    to_multi_app = sub
    source_variable = u
    variable = elemental_from_parent
  [../]
[]
