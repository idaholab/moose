[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [disp_x]
  []
[]

[AuxVariables]
  [vel_x]
  []
  [acc_x]
  []
[]

[Kernels]
  [ifx]
    type = InertialForce
    variable = disp_x
    density = 1
    use_displaced_mesh = false
  []
[]

[AuxKernels]
  [vel_x]
    type = TestNewmarkTI
    variable = vel_x
    displacement = disp_x
    first = true
    execute_on = 'INITIAL LINEAR TIMESTEP_BEGIN TIMESTEP_END'
  []
[]

[ICs]
  [current]
    type = ConstantIC
    variable = disp_x
    value = 0
    state = CURRENT
  []
  [old]
    type = ConstantIC
    variable = disp_x
    value = -1
    state = OLD
  []
[]

[Postprocessors]
  [disp_x]
    type = ElementAverageValue
    variable = disp_x
    execute_on = 'initial timestep_end'
  []
  [vel_x]
    type = ElementAverageValue
    variable = vel_x
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = CentralDifference
    solve_type = lumped
  []
  solve_type = LINEAR
  num_steps = 2
[]

[Outputs]
  csv = true
[]
