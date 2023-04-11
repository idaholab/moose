[GlobalParams]
  # This is suppressed in markers for adaptivity
  use_displaced_mesh = true
  # This is suppressed in the custom user object
  suppressed_param = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

# Mesh Marker System
[Adaptivity]
  [Markers]
    [boundary]
      type = BoundaryMarker
      next_to = right
      distance = 0.35
      mark = refine
    []
  []
  initial_marker = boundary
  initial_steps = 1
[]

[UserObjects]
  [tester]
    type = TestGlobalParamSuppression
  []
[]

[Outputs]
  exodus = true
[]
