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
    order = THIRD
    family = HERMITE
  [../]
  [./d]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  [../]
[]

[Kernels]
  [./CHSolid]
    type = CahnHilliardAniso
    variable = c
    f_name = F
    mob_name = M
  [../]
  [./CHInterface]
    type = CHInterfaceAniso
    variable = c
    mob_name = M
    kappa_name = kappa_c
  [../]
  [./ie_c]
    type = TimeDerivative
    variable = c
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

[Preconditioning]
  [./SMP]
   type = SMP
   full = true
  [../]
[]

[BCs]
  [./in_flux]
    type = NeumannBC
    variable = c
    boundary = top
    value = 0.2
  [../]
  [./out_flux]
    type = NeumannBC
    variable = c
    boundary = bottom
    value = -0.1
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

  dt = 0.01
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  print_perf_log = true
[]
