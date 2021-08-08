# Temperature is transferred between the fracture and matrix apps
[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 100
    xmin = 0
    xmax = 50.0
  []
[]

[Variables]
  [matrix_T]
  []
[]

[AuxVariables]
  [transferred_frac_T]
  []
[]

[Kernels]
  [dot]
    type = TimeDerivative
    variable = matrix_T
  []
  [matrix_diffusion]
    type = Diffusion
    variable = matrix_T
  []
  [fromFrac]
    type = PorousFlowHeatMassTransfer
    variable = matrix_T
    v = transferred_frac_T
    transfer_coefficient = 0.004
  []
[]

[Preconditioning]
  [entire_jacobian]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 100
  end_time = 100
[]


[Outputs]
  print_linear_residuals = false
[]
