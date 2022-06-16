[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  nz = 0
  xmax = 6
  ymax = 6
  zmax = 0
[]

[Variables]
  [./n]
    [./InitialCondition]
      type = RandomIC
      min = 0.0
      max = 0.1
    [../]
  [../]
  [./u]
    scaling = 1e2
  [../]
  [./v]
    scaling = 1e1
  [../]
[]

[AuxVariables]
  [./ed]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./edrff0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./edrff1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./edrff2]
    order = CONSTANT
    family = MONOMIAL
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

[AuxKernels]
  [./Energy_n]
    type = PFCEnergyDensity
    execute_on = 'initial timestep_end'
    variable = ed
    v = 'n u v'
  [../]

  [./Energy_rff0]
    type = PFCRFFEnergyDensity
    execute_on = 'initial timestep_end'
    variable = edrff0
    log_approach = tolerance
    v = 'n u v'
  [../]
  [./Energy_rff1]
    type = PFCRFFEnergyDensity
    execute_on = 'initial timestep_end'
    variable = edrff1
    log_approach = cancelation
    v = 'n u v'
  [../]
  [./Energy_rff2]
    type = PFCRFFEnergyDensity
    execute_on = 'initial timestep_end'
    variable = edrff2
    log_approach = expansion
    v = 'n u v'
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

[Postprocessors]
  [./Total_free_energy]
    type = PFCElementEnergyIntegral
    variable = ed
    execute_on = 'initial timestep_end'
  [../]
[]

[Preconditioning]
  active = 'SMP'
  [./SMP]
    type = SMP
    full = false
    off_diag_row = 'u n n v'
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
  # petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  # petsc_options_value = 'asm         101   preonly   lu      1'
  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu '

  l_max_its = 100
  l_tol = 1e-04
  nl_rel_tol = 1e-09
  nl_abs_tol = 1e-11

  num_steps = 1
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
