[Mesh]
  type = FileMesh
  file = crack_mesh.e
  uniform_refine = 0
[]

[Variables]
  [./u]
    block = 1
  [../]
  [./v]
    block = 1
  [../]
  [./c]
    block = 1
  [../]
  [./b]
    block = 1
  [../]
[]

[AuxVariables]
  [./resid_x]
    block = 1
  [../]
  [./resid_y]
    block = 1
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  [../]
[]

[Functions]
  [./tfunc]
    type = ParsedFunction
    value = t
  [../]
[]

[Kernels]
  [./pfbulk]
    type = PFFracBulkRate
    variable = c
    block = 1
    l = 0.08
    beta = b
    visco =1e-4
    gc_prop_var = 'gc_prop'
    G0_var = 'G0_pos'
    dG0_dstrain_var = 'dG0_pos_dstrain'
    disp_x = u
    disp_y = v
  [../]
  [./solid_x]
    type = StressDivergencePFFracTensors
    variable = u
    displacements = 'u v'
    component = 0
    block = 1
    save_in = resid_x
    c = c
  [../]
  [./solid_y]
    type = StressDivergencePFFracTensors
    variable = v
    displacements = 'u v'
    component = 1
    block = 1
    save_in = resid_y
    c = c
  [../]
  [./dcdt]
    type = TimeDerivative
    variable = c
    block = 1
  [../]
  [./pfintvar]
    type = PFFracIntVar
    variable = b
    block = 1
  [../]
  [./pfintcoupled]
    type = PFFracCoupledInterface
    variable = b
    c = c
    block = 1
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
    block = 1
  [../]
[]

[BCs]
  [./ydisp]
    type = FunctionPresetBC
    variable = v
    boundary = 2
    function = tfunc
  [../]
  [./yfix]
    type = PresetBC
    variable = v
    boundary = 1
    value = 0
  [../]
  [./xfix]
    type = PresetBC
    variable = u
    boundary = '1 2'
    value = 0
  [../]
[]

[Materials]
  [./pfbulkmat]
    type = PFFracBulkRateMaterial
    block = 1
    gc = 1e-3
  [../]
  [./elastic]
    type = LinearIsoElasticPFDamage
    block = 1
    c = c
    kdamage = 1e-8
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 1
    C_ijkl = '120.0 80.0'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 1
    displacements = 'u v'
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
  dtmin = 1e-4
  num_steps = 2
[]

[Outputs]
  exodus = true
  csv = true
  gnuplot = true
[]
