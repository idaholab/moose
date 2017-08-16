[GlobalParams]
  num_L = 5
  L_name_base = L
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 12
  ny = 12
  nz = 8
  xmax = 6
  ymax = 6
[]

[Variables]
  [./HHPFCRFFSplitVariables]
  [../]
[]

[AuxVariables]
  [./n]
  [../]
[]

[Kernels]
  [./HHPFCRFFSplitKernel]
    log_approach = expansion
    n_name = n
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
  type = Transient
  num_steps = 1
  dt = 0.1
  l_max_its = 50
  nl_max_its = 20
  petsc_options = '-pc_factor_shift_nonzero'
  petsc_options_iname = -pc_type
  petsc_options_value = lu
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
