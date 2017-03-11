[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = FileMesh
  file = crack_mesh.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./c]
  [../]
  [./b]
  [../]
[]

[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./pfbulk]
    type = AllenCahnPFFracture
    variable = c
    beta = b
  [../]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
  [../]
  [./solid_x]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_x
    component = 0
    c = c
  [../]
  [./solid_y]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_y
    component = 1
    c = c
  [../]
  [./dcdt]
    type = TimeDerivative
    variable = c
  [../]
  [./pfintvar]
    type = Reaction
    variable = b
  [../]
  [./pfintcoupled]
    type = LaplacianSplit
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
    function = 't'
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
    boundary = '3'
    value = 0
  [../]
[]

[Materials]
  [./pfbulkmat]
    type = GenericConstantMaterial
    prop_names = 'gc_prop l visco'
    prop_values = '1e-3 0.08 1e-4'
  [../]
  [./elastic]
    type = ComputeLinearElasticPFFractureStress
    c = c
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '120.0 80.0'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
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

  nl_rel_tol = 1e-9
  l_tol = 1e-4
  l_max_its = 100
  nl_max_its = 30

  dt = 5e-5
  num_steps = 1
[]

[Outputs]
  exodus = true
  print_perf_log = true
[]
