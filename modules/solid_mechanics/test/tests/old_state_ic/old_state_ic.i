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
  [old_disp_x]
  []
  [older_disp_x]
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
    execute_on = 'LINEAR TIMESTEP_END'
  []
  [acc_x]
    type = TestNewmarkTI
    variable = acc_x
    displacement = disp_x
    first = false
    execute_on = 'LINEAR TIMESTEP_END'
  []
  [old_disp_x]
    type = CopyValueAux
    variable = old_disp_x
    source = 'disp_x'
    state = OLD
    execute_on = 'initial timestep_end'
  []
  [older_disp_x]
    type = CopyValueAux
    variable = older_disp_x
    source = 'disp_x'
    state = OLDER
    execute_on = 'initial timestep_end'
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
  [older]
    type = ConstantIC
    variable = disp_x
    value = -3
    state = OLDER
  []
[]

[Postprocessors]
  [disp_x]
    type = ElementAverageValue
    variable = disp_x
    execute_on = 'initial timestep_end'
  []
  [old_disp_x]
    type = ElementAverageValue
    variable = old_disp_x
    execute_on = 'initial timestep_end'
  []
  [older_disp_x]
    type = ElementAverageValue
    variable = older_disp_x
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
  num_steps = 0
[]

[Outputs]
  csv = true
[]
