# ADHeatMassTransfer
# Transfer between two coupled variables u and v, exercising the diagonal (wrt u)
# and off-diagonal (wrt v) Jacobian entries.
# AD path: Jacobian verified against finite differences via PetscJacobianTester
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[ICs]
  [u]
    type = RandomIC
    variable = u
    min = 1
    max = 2
  []
  [v]
    type = RandomIC
    variable = v
    min = 1
    max = 2
  []
  [c]
    type = RandomIC
    variable = c
    min = 0.1
    max = 1
  []
[]

[Kernels]
  [u_dot]
    type = TimeDerivative
    variable = u
  []
  [v_dot]
    type = TimeDerivative
    variable = v
  []
  [transfer]
    type = ADPorousFlowHeatMassTransfer
    variable = u
    v = v
    transfer_coefficient = c
  []
[]

[AuxVariables]
  [c]
  []
[]

[Preconditioning]
  [check]
    type = SMP
    full = true
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

[Outputs]
  exodus = false
[]
