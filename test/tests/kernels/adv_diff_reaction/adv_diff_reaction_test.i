[Mesh]
  dim              = 2
  file             = Mesh24.e
[]

[Variables]
  active = 'phi'

  [./phi]
    order  = SECOND
    family = LAGRANGE
  [../]

[]

[Kernels]

  active = 'advection diffusion source'

  [./advection]
    type     = Advection0
    variable = phi
    Au       = 10.
    Bu       = -6.
    Cu       =  5.
    Av       = 10.
    Bv       =  8.
    Cv       = -1.
  [../]

  [./diffusion]
    type     = Diffusion0
    variable = phi
    Ak       = 10.
    Bk       = 0.1
    Ck       = 0.1
  [../]

  [./source]
    type     = ForcingFunctionXYZ0
    variable = phi
    omega0   = 2.
    A0       = 1.
    B0       = 1.2
    C0       = 0.8
    Au       = 10.
    Bu       = -6.
    Cu       =  5.
    Av       = 10.
    Bv       =  8.
    Cv       = -1.
    Ak       = 10.
    Bk       = 0.1
    Ck       = 0.1
  [../]

[]

[BCs]

  active = 'btm_sca rgt_sca top_sca lft_sca'

  [./btm_sca]
    type     = DirichletBCfuncXYZ0
    variable = phi
    boundary = 1
    omega0   = 2.
    A0       = 1.
    B0       = 1.2
    C0       = 0.8
  [../]

  [./rgt_sca]
    type     = DirichletBCfuncXYZ0
    variable = phi
    boundary = 2
    omega0   = 2.
    A0       = 1.
    B0       = 1.2
    C0       = 0.8
  [../]

  [./top_sca]
    type     = DirichletBCfuncXYZ0
    variable = phi
    boundary = 3
    omega0   = 2.
    A0       = 1.
    B0       = 1.2
    C0       = 0.8
  [../]

  [./lft_sca]
    type     = DirichletBCfuncXYZ0
    variable = phi
    boundary = 4
    omega0   = 2.
    A0       = 1.
    B0       = 1.2
    C0       = 0.8
  [../]

[]

[Executioner]
  type                 = Steady
  nl_rel_tol               = 1.e-10

  solve_type = 'PJFNK'

  petsc_options_iname  = '-pc_type -pc_factor_levels -pc_factor_mat_ordering_type'
  petsc_options_value  = 'ilu 20 rcm'
[]

[Outputs]
  exodus = true
[]
