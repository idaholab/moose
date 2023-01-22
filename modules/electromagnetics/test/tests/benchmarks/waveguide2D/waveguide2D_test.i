# Test for EMRobinBC in port and absorbing modes with simple electric plane wave
# 2D, vacuum-filled waveguide with conducting walls
# u^2 + k^2*u = 0, 0 < x < 80, 0 < y < 10, u: R -> C
# k = 2*pi*freq/c, freq = 20e6 Hz, c = 3e8 m/s

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = waveguide.msh
  []
[]

[Variables]
  [E_real]
    order = FIRST
    family = LAGRANGE
  []
  [E_imag]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  [inc_y]
    type = ParsedFunction
    expression = 'sin(pi * y / 10)'
  []
[]

[Kernels]
  [diffusion_real]
    type = Diffusion
    variable = E_real
  []
  [coeffField_real]
    type = ADMatReaction
    reaction_rate = kSquared
    variable = E_real
  []
  [diffusion_imaginary]
    type = Diffusion
    variable = E_imag
  []
  [coeffField_imaginary]
    type = ADMatReaction
    reaction_rate = kSquared
    variable = E_imag
  []
[]

[BCs]
  [top_real]
    type = DirichletBC
    value = 0
    variable = E_real
    boundary = top
  []
  [bottom_real]
    type = DirichletBC
    value = 0
    variable = E_real
    boundary = bottom
  []
  [port_real]
    type = EMRobinBC
    coeff_real = -0.27706242940220277  # -sqrt(k^2 - (pi/10)^2)
    sign = positive
    profile_func_real = inc_y
    profile_func_imag = 0
    field_real = E_real
    field_imaginary = E_imag
    variable = E_real
    component = real
    mode = port
    boundary = port
  []
  [exit_real]
    type = EMRobinBC
    coeff_real = 0.27706242940220277
    sign = negative
    field_real = E_real
    field_imaginary = E_imag
    variable = E_real
    component = real
    mode = absorbing
    boundary = exit
  []
  [top_imaginary]
    type = DirichletBC
    value = 0
    variable = E_imag
    boundary = top
  []
  [bottom_imaginary]
    type = DirichletBC
    value = 0
    variable = E_imag
    boundary = bottom
  []
  [port_imaginary]
    type = EMRobinBC
    coeff_real = -0.27706242940220277
    sign = positive
    profile_func_real = inc_y
    profile_func_imag = 0
    field_real = E_real
    field_imaginary = E_imag
    variable = E_imag
    component = imaginary
    mode = port
    boundary = port
  []
  [exit_imaginary]
    type = EMRobinBC
    coeff_real = 0.27706242940220277
    sign = negative
    field_real = E_real
    field_imaginary = E_imag
    variable = E_imag
    component = imaginary
    mode = absorbing
    boundary = exit
  []
[]

[Materials]
  [kSquared]
    type = ADParsedMaterial
    property_name = kSquared
    expression = '0.4188790204786391^2'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]
