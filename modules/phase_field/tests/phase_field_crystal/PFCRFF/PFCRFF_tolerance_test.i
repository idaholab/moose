[GlobalParams]
  num_L = 5
  L_name_base = L
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 12
  ny = 12
  xmax = 6
  ymax = 6
[]

[Variables]
  [./PFCRFFVariables]
  [../]
  [./n]
    [./InitialCondition]
      type = RandomIC
      max = 0.8
      min = 0.2
      seed = 12345
    [../]
  [../]
[]

[Kernels]
  [./PFCRFFKernel]
    n_name = n
    log_approach = tolerance
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
  [./PFC]
    type = PFCRFFMaterial
  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
[]

[Preconditioning]
  active = 'SMP'
  [./SMP]
    type = SMP
    full = true
  [../]
  [./FDP]
    type = FDP
    full = true
  [../]
[]

[Executioner]
  # petsc_options = '-snes_mf_operator -ksp_monitor'
  # petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  # petsc_options_value = 'hypre boomeramg 31'
  # petsc_options = '-pc_factor_shift_nonzero '
  # petsc_options_iname = -pc_type
  # petsc_options_value = lu
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         101   preonly   lu      5'
  type = Transient
  num_steps = 1
  dt = 0.1
  l_max_its = 50
  nl_max_its = 20
  solve_type = NEWTON
  l_tol = 1e-04
  nl_rel_tol = 1e-9
  scheme = bdf2
[]

[Outputs]
  exodus = true
[]

[ICs]
  active = ''
  [./density_IC]
    y2 = 10.5
    lc = 6
    y1 = 1.5
    min = .8
    max = .2
    x2 = 10.5
    crystal_structure = FCC
    variable = n
    x1 = 1.5
    type = PFCFreezingIC
  [../]
[]
