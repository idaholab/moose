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
  [old_disp_x]
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
  [old_disp_x]
    type = CopyValueAux
    variable = old_disp_x
    source = 'disp_x'
    state = OLD
    execute_on = 'initial timestep_end'
  []
[]

[ICs]
  [current]
    type = ConstantIC
    variable = disp_x
    value = 7
    state = CURRENT
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
