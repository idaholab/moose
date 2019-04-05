# problem: -(ku')' - c^2 * u' = 0 , 0 < x < L, u: R -> C
# u(x=0) = g0 , u'(x = L) = 2jcf(L)*exp(jcLf(L)) - jcf(L)u(x = L)
# c = a + jb , k = d + jh

[GlobalParams]
  length = 1
  coeff_real = 15
  coeff_imag = 7
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 100
[]

[Variables]
  [./u_real]
    order = FIRST
    family = LAGRANGE
  [../]
  [./u_imag]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./ASquaredMinusBSquared]
    type = ParsedFunction
    value = '15*15 - 7*7'
  [../]
  [./2TimesAB]
    type = ParsedFunction
    value = '2*15*7'
  [../]
  [./cos]
    type = ParsedFunction
    value = 'cos(0.5)'
  [../]
[]

[Kernels]
  [./laplacian_real]
    type = FuncDiffusion
    variable = u_real
  [../]
  [./coeffField_real]
    type = CoeffField
    func = ASquaredMinusBSquared
    variable = u_real
  [../]
  [./coupledField_real]
    type = CoupledCoeffField
    coupled_field = u_imag
    func = 2TimesAB
    sign = 1.0
    variable = u_real
  [../]
  [./laplacian_imag]
    type = FuncDiffusion
    variable = u_imag
  [../]
  [./coeffField_imag]
    type = CoeffField
    func = ASquaredMinusBSquared
    variable = u_imag
  [../]
  [./coupledField_imag]
    type = CoupledCoeffField
    coupled_field = u_real
    func = 2TimesAB
    sign = -1.0
    variable = u_imag
  [../]
[]

[BCs]
  [./left_real]
    type = DirichletBC
    value = 0
    boundary = left
    variable = u_real
  [../]
  [./left_imag]
    type = DirichletBC
    value = 1
    boundary = left
    variable = u_imag
  [../]
  [./right_real]
    type = ReflectionBC
    func_real = cos
    boundary = right
    component = real
    variable = u_real
    field_real = u_real
    field_imaginary = u_imag
    sign = -1.0
  [../]
  [./right_imag]
    type = ReflectionBC
    func_real = cos
    boundary = right
    component = imaginary
    variable = u_imag
    field_real = u_real
    field_imaginary = u_imag
    sign = -1.0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
