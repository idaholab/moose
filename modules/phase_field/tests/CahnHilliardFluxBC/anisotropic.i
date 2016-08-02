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
    type = SplitCHWResAniso
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
    type = CahnHilliardAnisoFluxBC
    variable = w
    boundary = top
    flux = '0 0.2 0'
    mob_name = M
    args = 'c d'
  [../]
  [./out_flux]
    type = CahnHilliardAnisoFluxBC
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

  # we assemble the variable dependent anosotropic mobility tensor form two
  # base tensors and their associated weights
  [./mob0]
    type = ConstantAnisotropicMobility
    tensor = '1 0 0  0 0.5 0  0 0 0'
    M_name = M0
  [../]
  [./mob1]
    type = ConstantAnisotropicMobility
    tensor = '0.5 0 0  0 1 0  0 0 0'
    M_name = M1
  [../]
  [./wgt0]
    type = DerivativeParsedMaterial
    f_name = w0
    function = 'c^2+d'
    args = 'c d'
  [../]
  [./wgt1]
    type = DerivativeParsedMaterial
    f_name = w1
    function = 'c+d^2'
    args = 'c d'
  [../]

  # assemble mobility tensor
  [./mob]
    type = CompositeMobilityTensor
    M_name = M
    tensors = 'M0 M1'
    weights = 'w0 w1'
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
  nl_max_its = 15
  nl_rel_tol = 1.0e-10
  num_steps = 2

  dt = 0.001
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  print_perf_log = true
[]
