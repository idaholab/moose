# problem: -(cu')' - k^2 * u = -F , 0 < x < L, u: R -> C
# u(x=0) = g0 , u(x=L) = gL
# k = a + jb
#     a = a(x) =  2 * (1 + x/L)
#     b = b(x) =      (1 + x/L)
# c = d + jh
#     d = d(x) = 12 * (1 + x/L)^2
#     h = h(x) =  4 * (1 + x/L)^2
# L = 10

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 10
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
  [./negative_ASquaredMinusBSquared]
    type = WaveCoeff
    k_real = '2 * (1 + x/10)'
    k_imag = '(1 + x/10)'
    eps_rel_real = 1
    eps_rel_imag = 0
    mu_rel_real = 1
    mu_rel_imag = 0
    coef = -1
    component = real
  [../]
  [./2TimesAB]
    type = WaveCoeff
    k_real = '2 * (1 + x/10)'
    k_imag = '(1 + x/10)'
    eps_rel_real = 1
    eps_rel_imag = 0
    mu_rel_real = 1
    mu_rel_imag = 0
    component = imaginary
  [../]
  [./d_func]
    type = ParsedFunction
    value = '12 * (1 + x/10)^2'
  [../]
  [./h_func]
    type = ParsedFunction
    value = '4 * (1 + x/10)^2'
  [../]
  [./RHS_real]
    type = MMSTestFunc
    L = 10
    g0_real = 1
    g0_imag = -1
    gL_real = 0
    gL_imag = 0
    component = real
  [../]
  [./RHS_imag]
    type = MMSTestFunc
    L = 10
    g0_real = 1
    g0_imag = -1
    gL_real = 0
    gL_imag = 0
    component = imaginary
  [../]
[]

[Kernels]
  [./laplacian_real]
    type = FuncDiffusion
    func = d_func
    variable = u_real
  [../]
  [./coupledLaplacian_real]
    type = CoupledFuncDiffusion
    func = h_func
    sign = -1.0
    coupled_field = u_imag
    variable = u_real
  [../]
  [./coeffField_real]
    type = ADFuncReaction
    func = negative_ASquaredMinusBSquared
    variable = u_real
  [../]
  [./coupledField_real]
    type = CoupledCoeffField
    v = u_imag
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
    type = FuncDiffusion
    func = d_func
    variable = u_imag
  [../]
  [./coupledLaplacian_imag]
    type = CoupledFuncDiffusion
    func = h_func
    sign = 1.0
    coupled_field = u_real
    variable = u_imag
  [../]
  [./coeffField_imag]
    type = ADFuncReaction
    func = negative_ASquaredMinusBSquared
    variable = u_imag
  [../]
  [./coupledField_imag]
    type = CoupledCoeffField
    v = u_real
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
