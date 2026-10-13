# Jacobian of current-configuration heat conduction with a conductivity varying with the current
# position, a volumetric source, and a boundary flux coupled to a displacement, with respect to the
# temperature and to nonlinear displacements

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  xmin = 1
  xmax = 2
  displacements = 'disp_x disp_y'
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
    use_displaced_mesh = true
  []
  [source]
    type = KokkosBodyForce
    variable = T
    value = 100
    use_displaced_mesh = true
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

[BCs]
  [flux]
    type = KokkosCoupledVarNeumannBC
    variable = T
    boundary = 'right top'
    v = disp_y
    use_displaced_mesh = true
  []
[]

[Materials]
  [conduction]
    type = KokkosHeatConductionMaterial
    thermal_conductivity = 10
    thermal_conductivity_gradient = '2 3 0'
    use_displaced_mesh = true
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
