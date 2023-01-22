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
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 100
  []
[]

[Variables]
  [u_real]
    order = FIRST
    family = LAGRANGE
  []
  [u_imag]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  [k_real]
    type = ParsedFunction
    expression = '2*(1 + x/10)'
  []
  [k_imag]
    type = ParsedFunction
    expression = '(1 + x/10)'
  []
  [d_func]
    type = ParsedFunction
    expression = '12 * (1 + x/10)^2'
  []
  [h_func]
    type = ParsedFunction
    expression = '4 * (1 + x/10)^2'
  []
  [negative_h_func]
    type = ParsedFunction
    expression = '-4 * (1 + x/10)^2'
  []
  [RHS_real]
    type = MMSTestFunc
    L = 10
    g0_real = 1
    g0_imag = -1
    gL_real = 0
    gL_imag = 0
    component = real
  []
  [RHS_imag]
    type = MMSTestFunc
    L = 10
    g0_real = 1
    g0_imag = -1
    gL_real = 0
    gL_imag = 0
    component = imaginary
  []
[]

[Materials]
  [k_real_mat]
    type = ADGenericFunctionMaterial
    prop_names = k_real_mat
    prop_values = k_real
  []
  [k_imag_mat]
    type = ADGenericFunctionMaterial
    prop_names = k_imag_mat
    prop_values = k_imag
  []
  [wave_equation_coefficient]
    type = WaveEquationCoefficient
    k_real = k_real_mat
    k_imag = k_imag_mat
    eps_rel_real = 1
    eps_rel_imag = 0
    mu_rel_real = 1
    mu_rel_imag = 0
  []
  [negative_wave_equation_coefficient_imaginary]
    type = ADParsedMaterial
    property_name = negative_wave_equation_coefficient_imaginary
    material_property_names = wave_equation_coefficient_imaginary
    expression = '-1 * wave_equation_coefficient_imaginary'
  []
[]

[Kernels]
  [laplacian_real]
    type = FunctionDiffusion
    function = d_func
    variable = u_real
  []
  [coupledLaplacian_real]
    type = FunctionDiffusion
    function = negative_h_func
    v = u_imag
    variable = u_real
  []
  [coeffField_real]
    type = ADMatReaction
    reaction_rate = wave_equation_coefficient_real
    variable = u_real
  []
  [coupledField_real]
    type = ADMatCoupledForce
    v = u_imag
    mat_prop_coef = negative_wave_equation_coefficient_imaginary
    variable = u_real
  []
  [bodyForce_real]
    type = BodyForce
    function = RHS_real
    variable = u_real
  []
  [laplacian_imag]
    type = FunctionDiffusion
    function = d_func
    variable = u_imag
  []
  [coupledLaplacian_imag]
    type = FunctionDiffusion
    function = h_func
    v = u_real
    variable = u_imag
  []
  [coeffField_imag]
    type = ADMatReaction
    reaction_rate = wave_equation_coefficient_real
    variable = u_imag
  []
  [coupledField_imag]
    type = ADMatCoupledForce
    v = u_real
    mat_prop_coef = wave_equation_coefficient_imaginary
    variable = u_imag
  []
  [bodyForce_imag]
    type = BodyForce
    function = RHS_imag
    variable = u_imag
  []
[]

[BCs]
  [left_real]
    type = DirichletBC
    value = 1
    boundary = left
    variable = u_real
  []
  [left_imag]
    type = DirichletBC
    value = -1
    boundary = left
    variable = u_imag
  []
  [right_real]
    type = DirichletBC
    value = 0
    boundary = right
    variable = u_real
  []
  [right_imag]
    type = DirichletBC
    value = 0
    boundary = right
    variable = u_imag
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
