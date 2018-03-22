# problem: (ku')' + c^2 * u' = 0 , 0 < x < L, u: R -> C
# u(x=0) = g0 , u(x=L) = gL
# c = a + jb , k = d + jh
# a = a(x), b = b(x)

[GlobalParams]
  L = 1
  d = 1
  h = 0
  g0_real = 1
  g0_imag = -1
  gL_real = 0
  gL_imag = 0
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
  [./a]
    type = ParsedFunction
    value = '5*(1 + x)^2'
  [../]
  [./b]
    type = ParsedFunction
    value = '(1 + x)^2'
  [../]
  [./aSquared]
    type = CompositeFunction
    functions = 'a a'
  [../]
  [./bSquared]
    type = CompositeFunction
    functions = 'b b'
  [../]
  [./ASquaredMinusBSquared]
    type = LinearCombinationFunction
    functions = 'aSquared bSquared'
    w = '1 -1'
  [../]
  [./2TimesAB]
    type = CompositeFunction
    functions = 'a b'
    scale_factor = 2
  [../]
  [./RHS_real]
    type = MMSTestFunc
    a = a
    b = b
    component = real
  [../]
  [./RHS_imag]
    type = MMSTestFunc
    a = a
    b = b
    component = imaginary
  [../]
[]

[Kernels]
  [./laplacian_real]
    type = CoeffDiffusion
    coefficient = 1
    variable = u_real
  [../]
  [./coupledLaplacian_real]
    type = CoupledCoeffDiffusion
    coefficient = 0
    sign = -1.0
    coupled_field = u_imag
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
  [./bodyForce_real]
    type = BodyForce
    function = RHS_real
    variable = u_real
  [../]
  [./laplacian_imag]
    type = CoeffDiffusion
    coefficient = 1
    variable = u_imag
  [../]
  [./coupledLaplacian_imag]
    type = CoupledCoeffDiffusion
    coefficient = 0
    sign = 1.0
    coupled_field = u_real
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
  [./bodyForce_imag]
    type = BodyForce
    function = RHS_imag
    variable = u_imag
  [../]
[]

[BCs]
  [./left_real]
    type = DirichletBC
    value = 1
    boundary = left
    variable = u_real
  [../]
  [./left_imag]
    type = DirichletBC
    value = -1
    boundary = left
    variable = u_imag
  [../]
  [./right_real]
    type = DirichletBC
    value = 0
    boundary = right
    variable = u_real
  [../]
  [./right_imag]
    type = DirichletBC
    value = 0
    boundary = right
    variable = u_imag
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
