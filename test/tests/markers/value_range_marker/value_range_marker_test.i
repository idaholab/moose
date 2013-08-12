[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  zmax = 0
  elem_type = QUAD4
  # This test currently diffs when run in parallel with ParallelMesh enabled,
  # most likely due to the fact that CONSTANT MONOMIALS are currently not written
  # out correctly in this case.  For more information, see #2122.
  distribution = serial
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
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
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Adaptivity]
  [./Markers]
    [./marker]
      type = ValueRangeMarker
      lower_bound = 0.3
      upper_bound = 0.6
      buffer_size = 0.1
      variable = u
      third_state = DO_NOTHING
    [../]
    [./inverted_marker]
      type = ValueRangeMarker
      invert = true
      lower_bound = 0.3
      upper_bound = 0.6
      buffer_size = 0.1
      variable = u
      third_state = DO_NOTHING
    [../]
  [../]
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

