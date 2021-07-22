# Heat energy from this fracture app is transferred to the matrix app
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
  [heat_from_frac]
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
    type = CoupledForce
    variable = matrix_T
    v = heat_from_frac
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
