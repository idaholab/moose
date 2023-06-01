# problem: -(ku')' - c^2 * u' = 0 , 0 < x < L, u: R -> C
# u(x=0) = g0 , u'(x = L) = 2jcf(L)*exp(jcLf(L)) - jcf(L)u(x = L)
# c = a + jb , k = d + jh

[GlobalParams]
  coeff_real = 15
  coeff_imag = 7
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 1
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
  [cos]
    type = ParsedFunction
    expression = 'cos(0.5)'
  []
[]

[Materials]
  [ASquaredMinusBSquared]
    type = ADParsedMaterial
    property_name = ASquaredMinusBSquared
    expression = '(15*15 - 7*7)'
  []
  [2TimesAB]
    type = ADParsedMaterial
    property_name = 2TimesAB
    expression = '2*15*7'
  []
  [negative_2TimesAB]
    type = ADParsedMaterial
    property_name = negative_2TimesAB
    expression = '-2*15*7'
  []
[]

[Kernels]
  [laplacian_real]
    type = Diffusion
    variable = u_real
  []
  [coeffField_real]
    type = ADMatReaction
    reaction_rate = ASquaredMinusBSquared
    variable = u_real
  []
  [coupledField_real]
    type = ADMatCoupledForce
    v = u_imag
    mat_prop_coef = negative_2TimesAB
    variable = u_real
  []
  [laplacian_imag]
    type = Diffusion
    variable = u_imag
  []
  [coeffField_imag]
    type = ADMatReaction
    reaction_rate = ASquaredMinusBSquared
    variable = u_imag
  []
  [coupledField_imag]
    type = ADMatCoupledForce
    v = u_real
    mat_prop_coef = 2TimesAB
    variable = u_imag
  []
[]

[BCs]
  [left_real]
    type = DirichletBC
    value = 0
    boundary = left
    variable = u_real
  []
  [left_imag]
    type = DirichletBC
    value = 1
    boundary = left
    variable = u_imag
  []
  [right_real]
    type = EMRobinBC
    func_real = cos
    boundary = right
    component = real
    variable = u_real
    field_real = u_real
    field_imaginary = u_imag
    sign = negative
  []
  [right_imag]
    type = EMRobinBC
    func_real = cos
    boundary = right
    component = imaginary
    variable = u_imag
    field_real = u_real
    field_imaginary = u_imag
    sign = negative
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
