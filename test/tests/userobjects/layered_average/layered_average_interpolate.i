[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  # This test currently diffs when run in parallel with ParallelMesh enabled,
  # most likely due to the fact that CONSTANT MONOMIALS are currently not written
  # out correctly in this case.  For more information, see #2122.
  distribution = serial
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./layered_average]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./nodal_layered_average]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./layered_average]
    type = SpatialUserObjectAux
    variable = layered_average
    execute_on = timestep
    user_object = average
  [../]
  [./nodal_layered_average]
    type = SpatialUserObjectAux
    variable = nodal_layered_average
    user_object = average
  [../]
[]

[BCs]
  [./top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 1
  [../]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  [../]
[]

[UserObjects]
  [./average]
    type = LayeredAverage
    variable = u
    direction = y
    num_layers = 19
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

