# 1D metal backed dielectric slab benchmark (electric field edition)
# Based on Section 3.4 of Jin, "The Finite Element Method in Electromagnetics, 3rd Ed."
# frequency = 20 MHz
# eps_R = 4 + (2 - j0.1)(1 - x/L)^2
# mu_R = 2 - j0.1
# L = 5 * wavelength

k =  0.41887902047863906 # 2 * pi * 20e6 / 3e8
L =  75 # = 5 * c / freq. (in m)
E0 = 1 # magnitude of the incident field (in V/m)

[GlobalParams]
  theta = 0
[]

[Mesh]
  [./slab]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 100
    xmin = 0
    xmax = ${L}
  [../]
  [./rename]
    type = RenameBoundaryGenerator
    input = slab
    old_boundary = 'left right'
    new_boundary = 'metal vacuum'
  [../]
[]

[Variables]
  [./E_real]
    order = FIRST
    family = LAGRANGE
  [../]
  [./E_imag]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./coeff_real]
    type = JinSlabCoeffFunc
    k = ${k}
    length = ${L}
    component = real
  [../]
  [./coeff_imag]
    type = JinSlabCoeffFunc
    k = ${k}
    length = ${L}
    component = imaginary
  [../]
  [./cosTheta]
    type = CosTheta
  [../]
[]

[Kernels]
  [./diffusion_real]
    type = Diffusion
    variable = E_real
  [../]
  [./field_real]
    type = CoeffField
    func = coeff_real
    coeff = -1
    variable = E_real
  [../]
  [./coupled_real]
    type = CoupledCoeffField
    func = coeff_imag
    coupled_field = E_imag
    variable = E_real
  [../]
  [./diffusion_imag]
    type = Diffusion
    variable = E_imag
  [../]
  [./field_imag]
    type = CoeffField
    func = coeff_real
    coeff = -1
    variable = E_imag
  [../]
  [./coupled_imag]
    type = CoupledCoeffField
    func = coeff_imag
    sign = -1
    coupled_field = E_real
    variable = E_imag
  [../]
[]

[BCs]
  [./metal_real]
    type = DirichletBC
    value = 0
    variable = E_real
    boundary = metal
  [../]
  [./metal_imag]
    type = DirichletBC
    value = 0
    variable = E_imag
    boundary = metal
  [../]
  [./vacuum_real]
    type = EMRobinBC
    coeff_real = ${k}
    func_real = cosTheta
    profile_func_real = ${E0}
    boundary = vacuum
    component = real
    field_real = E_real
    field_imaginary = E_imag
    variable = E_real
    sign = -1
  [../]
  [./vacuum_imag]
    type = EMRobinBC
    coeff_real = ${k}
    func_real = cosTheta
    profile_func_real = ${E0}
    boundary = vacuum
    component = imaginary
    field_real = E_real
    field_imaginary = E_imag
    variable = E_imag
    sign = -1
  [../]
[]

[Postprocessors]
  [./reflection_coefficient]
    type = ReflectionCoefficient
    k = ${k}
    length = ${L}
    incoming_field_magnitude = ${E0}
    field_real = E_real
    field_imag = E_imag
    boundary = vacuum
    outputs = 'csv console'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = false
  csv = true
  print_linear_residuals = true
[]
