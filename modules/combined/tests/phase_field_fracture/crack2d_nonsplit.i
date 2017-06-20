[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 5
  ymax = 0.5
  second_order = true
[]

[MeshModifiers]
  [./noncrack]
    type = BoundingBoxNodeSet
    new_boundary = noncrack
    bottom_left = '0.5 0 0'
    top_right = '1 0 0'
  [../]
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  order = SECOND
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./c]
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
  [./pfbulk]
    type = PFFractureBulkRate
    variable = c
    width = 0.08
    viscosity = 1e-1
    gc = 'gc_prop'
    G0 = 'G0_pos'
    dG0_dstrain = 'dG0_pos_dstrain'
  [../]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
    save_in = 'resid_x resid_y'
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
    boundary = top
    function = tfunc
  [../]
  [./yfix]
    type = PresetBC
    variable = disp_y
    boundary = noncrack
    value = 0
  [../]
  [./xfix]
    type = PresetBC
    variable = disp_x
    boundary = top
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
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK

  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      lu           1'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  l_max_its = 35
  nl_max_its = 10

  dt = 1e-4
  num_steps = 2
[]

[Outputs]
  [./out]
    type = Exodus
    refinements = 1
  [../]
[]
