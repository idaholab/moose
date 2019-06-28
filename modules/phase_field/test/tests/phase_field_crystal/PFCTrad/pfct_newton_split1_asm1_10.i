[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  xmax = 8
  ymax = 8
[]

[Variables]
  [./n]
    [./InitialCondition]
      type = RandomIC
      min = -1
      max = 4
    [../]
  [../]
  [./u]
    scaling = 1e2
  [../]
  [./v]
    scaling = 1e1
  [../]
[]

[Kernels]
  [./ndot]
    type = TimeDerivative
    variable = n
  [../]
  [./n_bulk]
    type = CHBulkPFCTrad
    variable = n
  [../]
  [./u_term]
    type = MatDiffusion
    variable = n
    v = u
    diffusivity = C2
  [../]
  [./v_term]
    type = MatDiffusion
    variable = n
    v = v
    diffusivity = C4
  [../]
  [./u_rctn]
    type = Reaction
    variable = u
  [../]
  [./u_gradn]
    type = LaplacianSplit
    variable = u
    c = n
  [../]
  [./v_rctn]
    type = Reaction
    variable = v
  [../]
  [./v_gradu]
    type = LaplacianSplit
    variable = v
    c = u
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./PFCTrad]
    type = PFCTradMaterial
    order = 4
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  l_max_its = 100
  l_tol = 1e-04
  nl_rel_tol = 1e-09
  nl_abs_tol = 1e-11

  splitting = 'nuv'
  petsc_options = '-snes_view'

  num_steps = 2
  dt = 0.1
[]

[Splits]
  [./nuv]
    splitting       = 'v nu'
    splitting_type  = schur
    schur_type      = full
    schur_pre       = Sp
    #petsc_options = '-dm_view'
  [../]
  [./nu]
    vars = 'n u'
    petsc_options = '-ksp_monitor'
    petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_asm_nblocks -pc_asm_overlap  -sub_pc_type'
    petsc_options_value =  '              101      asm              10               1            lu'
  [../]
  [./v]
    vars = 'v'
    #petsc_options = '-ksp_monitor'
    petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
    petsc_options_value =  'asm         101   preonly   lu      0'
    #full = true
  [../]
[]

[Outputs]
  exodus = true
[]
