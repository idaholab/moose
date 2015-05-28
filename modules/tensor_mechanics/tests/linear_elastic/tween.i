[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[Functions]
  [./prefactor]
    type = ParsedFunction

    # spatial variation for testing purposes
    value = 'if(y>0.5,20,1)'

    # fade in (or tween) an elasticity tensor at the start of the simulation
    #value = 'if(t<1, max(t,0.1), 1.0)'
  [../]
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
[]

[Kernels]
  [./TensorMechanics]
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

[AuxVariables]
  [./e11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e22_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]
[AuxKernels]
  [./matl_e11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = e11_aux
  [../]
  [./matl_e22]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = e22_aux
  [../]
[]

[Materials]
  # matrix
  [./material_1]
    type = LinearElasticMaterial
    block = 0
    disp_y = disp_y
    disp_x = disp_x
    C_ijkl = '1 1'
    elasticity_tensor_prefactor = prefactor
    fill_method = symmetric_isotropic
  [../]
[]

[BCs]
  [./bottom_y]
    type = PresetBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./top_y]
    type = PresetBC
    variable = disp_y
    boundary = 'top'
    value = 0.1
  [../]
  [./left_x]
    type = PresetBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
  [./right_x]
    type = PresetBC
    variable = disp_x
    boundary = 'right'
    value = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'

  l_max_its = 20
  nl_max_its = 10
  l_tol = 1.0e-7
  nl_rel_tol = 1.0e-12
[]

[Outputs]
  exodus = true
  output_on = 'timestep_end'
  [./console]
    type = Console
    perf_log = true
    output_on = 'initial timestep_end failed nonlinear'
  [../]
[]
