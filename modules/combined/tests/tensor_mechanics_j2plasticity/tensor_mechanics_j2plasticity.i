[Mesh]
  type = GeneratedMesh
  elem_type = HEX8
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin=0.0
  xmax=1.0
  ymin=0.0
  ymax=1.0
  zmin=0.0
  zmax=1.0
  
  [./ExtraNodesets]
    [./cnode]
      coord = '0.0 0.0 0.0'
      id = 6
    [../]
  [../]

  [./ExtraNodesets]
    [./snode]
      coord = '1.0 0.0 0.0'
      id = 7
    [../]
  [../]
[]

[Variables]
  [./x_disp]
    order = FIRST
    family = LAGRANGE
  [../]
 
 [./y_disp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./z_disp]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[TensorMechanics]
  [./solid]
    disp_x = x_disp
    disp_y = y_disp
    disp_z = z_disp
  [../]
[]



[Materials]
  active='felastic'
  [./felastic]
    type = FiniteStrainPlasticMaterial
    block=0
    all_21 = false	
    disp_x = x_disp	
    disp_y = y_disp
    disp_z = z_disp
    C_ijkl = '2.827e5 1.21e5 1.21e5 2.827e5 1.21e5 2.827e5 0.808e5 0.808e5 0.808e5'
    yield_stress=300.
  [../]
[]

[Functions]

 [./topfunc]
   type = ParsedFunction
   value = 't'
 [../]

[]

[BCs]
  [./bottom3]
    type = PresetBC
    variable = z_disp
    boundary = 0
    value = 0.0
  [../]
  [./top]
    type = FunctionPresetBC
    variable = z_disp
    boundary = 5
    function = topfunc
  [../]
  [./corner1]
    type = PresetBC
    variable = x_disp
    boundary = 6
    value = 0.0
  [../]
  [./corner2]
    type = PresetBC
    variable = y_disp
    boundary = 6
    value = 0.0
  [../]
  [./corner3]
    type = PresetBC
    variable = z_disp
    boundary = 6
    value = 0.0
  [../]

  [./side1]
    type = PresetBC
    variable = y_disp
    boundary = 7
    value = 0.0
  [../]
  [./side2]
    type = PresetBC
    variable = z_disp
    boundary = 7
    value = 0.0
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  # AuxKernels
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 1
    index_j = 1
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 2
    index_j = 2
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 3
    index_j = 3
  [../]
[]

[Preconditioning]
#active = ''

  [./SMP]
   type = SMP
   full=true
  [../]
[]

[Executioner]
  start_time = 0.0
  end_time=1.0
  dt=0.1
  dtmax=1
  dtmin=0.1
  type = Transient
  petsc_options = '-snes_mf_operator'
  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomerang
  nl_abs_tol = 1e-10 
[]


[Output]
  file_base = out
  interval = 1
  output_initial = true
  exodus = true
  perf_log = true
[]



