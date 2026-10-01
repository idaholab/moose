[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  displacements = 'disp_x'
[]

[Problem]
  nl_sys_names = 'reference kokkos'
[]

[AuxVariables]
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = x
    []
  []
[]

[Variables]
  [reference]
    solver_sys = reference
  []
  [kokkos]
    solver_sys = kokkos
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = reference
    use_displaced_mesh = true
  []
  [source]
    type = BodyForce
    variable = reference
    use_displaced_mesh = true
  []
  [kokkos_diffusion]
    type = KokkosDiffusion
    variable = kokkos
    use_displaced_mesh = true
  []
  [kokkos_source]
    type = KokkosBodyForce
    variable = kokkos
    use_displaced_mesh = true
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = reference
    boundary = left
    value = 0
    use_displaced_mesh = true
  []
  [right]
    type = DirichletBC
    variable = reference
    boundary = right
    value = 0
    use_displaced_mesh = true
  []
  [kokkos_left]
    type = KokkosDirichletBC
    variable = kokkos
    boundary = left
    value = 0
    use_displaced_mesh = true
  []
  [kokkos_right]
    type = KokkosDirichletBC
    variable = kokkos
    boundary = right
    value = 0
    use_displaced_mesh = true
  []
[]

[Postprocessors]
  [reference_integral]
    type = ElementIntegralVariablePostprocessor
    variable = reference
    use_displaced_mesh = true
  []
  [kokkos_integral]
    type = KokkosElementIntegralVariablePostprocessor
    variable = kokkos
    use_displaced_mesh = true
  []
  [difference]
    type = ElementL2Difference
    variable = reference
    other_variable = kokkos
    use_displaced_mesh = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  system_names = 'reference kokkos'
[]

[Outputs]
  csv = true
[]
