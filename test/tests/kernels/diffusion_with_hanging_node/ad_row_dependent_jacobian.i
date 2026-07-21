[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [row_dependent]
    type = ADRowDependentTestKernel
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
    preset = false
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
    preset = false
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Adaptivity]
  marker = box
  max_h_level = 1
  initial_steps = 1

  [Markers]
    [box]
      type = BoxMarker
      bottom_left = '0.5 0 0'
      top_right = '1 1 0'
      inside = refine
      outside = do_nothing
    []
  []
[]
