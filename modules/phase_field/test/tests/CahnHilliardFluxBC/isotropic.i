[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 5.0
  ymax = 5.0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]
  [./d]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    coupled_groups = 'c,w'
  [../]
[]

[Kernels]
  [./cres]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
  [../]
  [./wres]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]

  [./diff]
    type = MatDiffusion
    D_name = 10.0
    variable = d
  [../]
  [./time2]
    type = TimeDerivative
    variable = d
  [../]
[]

[BCs]
  [./in_flux]
    type = CahnHilliardFluxBC
    variable = w
    boundary = top
    flux = '0 0.2 0'
    mob_name = M
    args = 'c d'
  [../]
  [./out_flux]
    type = CahnHilliardFluxBC
    variable = w
    boundary = bottom
    flux = '0 0.1 0'
    mob_name = M
    args = 'c d'
  [../]

  [./dirichlet_left]
    type = DirichletBC
    boundary = left
    variable = d
    value = 0
  [../]
  [./dirichlet_right]
    type = DirichletBC
    boundary = right
    variable = d
    value = 2
  [../]
[]

[Materials]
  [./constant]
    type = GenericConstantMaterial
    prop_names  = 'kappa_c'
    prop_values = '2.0'
  [../]
  [./mob]
    type = DerivativeParsedMaterial
    f_name = M
    function = 'c^2+d+0.1'
    args = 'c d'
  [../]
  [./F]
    type = DerivativeParsedMaterial
    f_name = F
    function = 'c^2'
    args = 'c'
  [../]
[]

[Postprocessors]
  [./total_solute]
    type = ElementIntegralVariablePostprocessor
    variable = c
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'BDF2'

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm       ilu'

  l_max_its = 30
  l_tol = 1.0e-3
  nl_max_its = 10
  nl_rel_tol = 1.0e-12
  num_steps = 2

  dt = 0.1
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  print_perf_log = true
[]
