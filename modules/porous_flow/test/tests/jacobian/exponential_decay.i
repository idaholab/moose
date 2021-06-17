# ExponentialDecay
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
  [exp_decay]
    type = PorousFlowExponentialDecay
    variable = u
    rate = rate
    reference = reference
  []
[]

[AuxVariables]
  [rate]
  []
  [reference]
  []
[]

[ICs]
  [rate]
    type = RandomIC
    variable = rate
    min = -1
    max = 1
  []
  [reference]
    type = RandomIC
    variable = reference
    min = 1
    max = 2
  []
[]

[Preconditioning]
  [check]
    type = SMP
    full = true
    #petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]
