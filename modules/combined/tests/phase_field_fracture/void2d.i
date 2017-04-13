[Mesh]
  type = FileMesh
  file = void2d_mesh.xda
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
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
    type = PFFracBulkRate
    variable = c
    l = 0.01
    beta = b
    visco = 1e-1
    gc_prop_var = 'gc_prop'
    G0_var = 'G0_pos'
    dG0_dstrain_var = 'dG0_pos_dstrain'
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
  [./pfintvar]
    type = Reaction
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
    boundary = top
    function = tfunc
  [../]
  [./yfix]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./xfix]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
[]

[Functions]
  [./void_prop_func]
    type = ParsedFunction
    value = 'rad:=0.2;m:=50;r:=sqrt(x^2+y^2);1-exp(-(r/rad)^m)+1e-8'
  [../]
  [./gb_prop_func]
    type = ParsedFunction
    value = 'rad:=0.2;thk:=0.05;m:=50;sgnx:=1-exp(-(x/rad)^m);v:=sgnx*exp(-(y/thk)^m);0.005*(1-v)+0.001*v'
  [../]
[../]

[Materials]
  [./pfbulkmat]
    type = PFFracBulkRateMaterial
    block = 0
    function = gb_prop_func
  [../]
  [./elastic]
    type = LinearIsoElasticPFDamage
    block = 0
    c = c
    kdamage = 1e-8
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '120.0 80.0'
    fill_method = symmetric_isotropic
    elasticity_tensor_prefactor = void_prop_func
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
  [../]
[]

[Postprocessors]
  [./resid_x]
    type = NodalSum
    variable = resid_x
    boundary = left
  [../]
  [./resid_y]
    type = NodalSum
    variable = resid_y
    boundary = top
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

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_max_its = 15
  nl_max_its = 10

  dt = 1e-4
  num_steps = 2
[]

[Outputs]
  exodus = true
  csv = true
  gnuplot = true
[]
