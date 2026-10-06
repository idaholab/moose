# Jacobian of the current-configuration heat conduction and source terms with respect to the
# temperature and to nonlinear displacements. No displaced mesh is created.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  xmin = 1
  xmax = 2
[]

[Variables]
  [T]
    [InitialCondition]
      type = FunctionIC
      function = 'x * x + 3 * x * y'
    []
  []
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

[Kernels]
  [conduction]
    type = KokkosHeatConduction
    variable = T
    displacements = 'disp_x disp_y'
  []
  [source]
    type = KokkosBodyForce
    variable = T
    value = 100
    displacements = 'disp_x disp_y'
  []
  [disp_x]
    type = KokkosDiffusion
    variable = disp_x
  []
  [disp_y]
    type = KokkosDiffusion
    variable = disp_y
  []
[]

[Materials]
  [conduction]
    type = KokkosHeatConductionMaterial
    thermal_conductivity = 10
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]
