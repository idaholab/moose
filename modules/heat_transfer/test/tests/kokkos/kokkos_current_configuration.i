# Heat conduction with a volumetric source posed on the current configuration. The Kokkos kernels
# couple the displacements and pull the weak form back to the reference configuration; the
# non-Kokkos kernels integrate on the displaced mesh. The two discrete solutions agree to roundoff.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
  xmin = 1
  xmax = 2
  displacements = 'disp_x disp_y'
[]

[Problem]
  nl_sys_names = 'reference kokkos'
[]

[AuxVariables]
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = '0.1 * x * y + 0.2 * x'
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = '0.2 * x * x + 0.5 * y'
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
  [conduction]
    type = HeatConduction
    variable = reference
    thermal_conductivity = k_reference
  []
  [source]
    type = BodyForce
    variable = reference
    value = 100
    use_displaced_mesh = true
  []
  [kokkos_conduction]
    type = KokkosHeatConduction
    variable = kokkos
    thermal_conductivity = k_kokkos
    displacements = 'disp_x disp_y'
  []
  [kokkos_source]
    type = KokkosBodyForce
    variable = kokkos
    value = 100
    displacements = 'disp_x disp_y'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = reference
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = reference
    boundary = right
    value = 1
  []
  [kokkos_left]
    type = KokkosDirichletBC
    variable = kokkos
    boundary = left
    value = 0
  []
  [kokkos_right]
    type = KokkosDirichletBC
    variable = kokkos
    boundary = right
    value = 1
  []
[]

[Materials]
  [k_reference]
    type = GenericConstantMaterial
    prop_names = 'k_reference'
    prop_values = 10
  []
  [k_kokkos]
    type = KokkosGenericConstantMaterial
    prop_names = 'k_kokkos'
    prop_values = 10
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
    displacements = 'disp_x disp_y'
  []
  [difference]
    type = ElementL2Difference
    variable = reference
    other_variable = kokkos
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  system_names = 'reference kokkos'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
[]

[Outputs]
  csv = true
[]
