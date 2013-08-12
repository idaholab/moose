[Mesh]
  file = 3d_thermal_contact_tet.e
  dim = 2
  construct_side_list_from_node_list = true
  # This test will not work in parallel with ParallelMesh enabled
  # due to a bug in the GeometricSearch system.
  distribution = serial
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./gap_distance]
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
    boundary = leftleft
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = rightright
    value = 1
  [../]
[]

[AuxBCs]
  [./distance]
    type = PenetrationAux
    variable = gap_distance
    boundary = leftright
    paired_boundary = rightleft
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = -snes_mf_operator
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

