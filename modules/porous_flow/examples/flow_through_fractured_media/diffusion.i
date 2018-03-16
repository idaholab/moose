[Mesh]
  file = diffusion_1.e # or diffusion_5.e or diffusion_fine.e
[]

[Variables]
  [./T]
  [../]
[]

[BCs]
  [./left]
    type = PresetBC
    boundary = 2
    variable = T
    value = 1
  [../]
[]

[Kernels]
  [./dot]
    type = TimeDerivative
    variable = T
  [../]
  [./fracture_diffusion]
    type = AnisotropicDiffusion
    block = 1
    tensor_coeff = '1 0 0  0 1 0  0 0 1'
    variable = T
  [../]
  [./matrix_diffusion]
    type = AnisotropicDiffusion
    block = '2 3'
    tensor_coeff = '0 0 0  0 0 0  0 0 0'
    variable = T
  [../]
[]

[Preconditioning]
  [./entire_jacobian]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 10
  end_time = 100
[]

[Outputs]
  print_linear_residuals = false
  exodus = true
[]


