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
    order = FOURTH
  [../]
[]

[Preconditioning]
  active = 'SMP'
  [./SMP]
    type = SMP
    full = false
    off_diag_row    = 'u n n v'
    off_diag_column = 'n u v u'
  [../]
  [./FDP]
    type = FDP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  # petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  # petsc_options_value = 'hypre boomeramg 101'
  # petsc_options_iname = -pc_type
  # petsc_options_value = lu
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         101   preonly   lu      5'

  l_max_its = 100
  l_tol = 1e-04
  nl_rel_tol = 1e-09
  nl_abs_tol = 1e-11

  num_steps = 2
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
