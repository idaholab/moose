[Mesh]
  type = FileMesh
  file = crack_mesh.e
  uniform_refine = 0
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./c]
  [../]
  [./b]
  [../]
[]

[AuxVariables]
  [./resid_x]
  [../]
  [./resid_y]
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./tfunc]
    type = ParsedFunction
    value = t
  [../]
[]

[Kernels]
  [./pfbulk_x]
    type = PFFracBulkRate
    variable = c
    l = 0.08
    beta = b
    visco =1e-4
    gc_prop_var = 'gc_prop'
    G0_var = 'G0_pos'
    dG0_dstrain_var = 'dG0_pos_dstrain'
    displacements = 'disp_x disp_y'
    component = 0
  [../]
  [./pfbulk_y]
    type = PFFracBulkRate
    variable = c
    l = 0.08
    beta = b
    visco =1e-4
    gc_prop_var = 'gc_prop'
    G0_var = 'G0_pos'
    dG0_dstrain_var = 'dG0_pos_dstrain'
    displacements = 'disp_x disp_y'
    component = 1
  [../]
  [./solid_x]
    type = StressDivergencePFFracTensors
    variable = disp_x
    displacements = 'disp_x disp_y'
    component = 0
    save_in = resid_x
    c = c
  [../]
  [./solid_y]
    type = StressDivergencePFFracTensors
    variable = disp_y
    displacements = 'disp_x disp_y'
    component = 1
    save_in = resid_y
    c = c
  [../]
  [./dcdt]
    type = TimeDerivative
    variable = c
  [../]
  [./pfintvar]
    type = PFFracIntVar
    variable = b
  [../]
  [./pfintcoupled]
    type = PFFracCoupledInterface
    variable = b
    c = c
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = stress
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./ydisp]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 2
    function = tfunc
  [../]
  [./yfix]
    type = PresetBC
    variable = disp_y
    boundary = 1
    value = 0
  [../]
  [./xfix]
    type = PresetBC
    variable = disp_x
    boundary = '1 2'
    value = 0
  [../]
[]

[Materials]
  [./pfbulkmat]
    type = PFFracBulkRateMaterial
    gc = 1e-3
  [../]
  [./elastic]
    type = LinearIsoElasticPFDamage
    c = c
    kdamage = 1e-8
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '120.0 80.0'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y'
  [../]
[]

[Postprocessors]
  [./resid_x]
    type = NodalSum
    variable = resid_x
    boundary = 2
  [../]
  [./resid_y]
    type = NodalSum
    variable = resid_y
    boundary = 2
  [../]
[]

[Preconditioning]
  active = 'smp'
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  preonly       lu           1'

  nl_rel_tol = 1e-8
  l_max_its = 10
  nl_max_its = 10

  dt = 1e-4
  dtmin = 1e-5
  num_steps = 2
[]

[Outputs]
  file_base = crack_2d_new
  exodus = true
  csv = true
  gnuplot = true
[]
