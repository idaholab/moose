###########################################################
# This is a test of the Mesh Marker System. It marks
# elements with flags indicating whether they should be
# refined, coarsened, or left alone. This system
# has the ability to use the Mesh Indicator System.
#
# @Requirement F2.50
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 50
  ny = 50
  nz = 50
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 10

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

# Mesh Marker System
[Adaptivity]
  marker = uniform
  cycles_per_step = 1

  # set this to true to skip the call to refine_and_coarsen_elements()
  check_markers_before_adapting_mesh = false

  [./Markers]
    [./uniform]
      type = UniformMarker
      mark = do_nothing
    [../]
  [../]
[]

[Outputs]
  print_perf_log = true
[]
