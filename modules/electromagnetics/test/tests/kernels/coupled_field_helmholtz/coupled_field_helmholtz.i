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
    value = '15^2 - 7^2'
  [../]
  [./2TimesAB]
    type = ParsedFunction
    value = '2*15*7'
  [../]
[]

[Kernels]
  [./laplacian_real]
    type = CoeffDiffusion
    variable = u_real
  [../]
  [./kSquaredEpsField_real]
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
    type = CoeffDiffusion
    variable = u_imag
  [../]
  [./kSquaredEpsField_imag]
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
    type = DirichletBC
    value = 0
    boundary = right
    variable = u_real
  [../]
  [./right_imag]
    type = DirichletBC
    value = -1
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
