[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 56
  nz = 0
  xmin = 0
  xmax = 200
  ymin = 0
  ymax = 112
  zmin = 0
  zmax = 0
[]

[GlobalParams]
  op_num = 6
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./bnds]
  [../]
[]

[UserObjects]
  [./circle_IC]
    type = PolycrystalCircles
    radii =       '22  22  30  22  22  22  22  22 '
    x_positions = '34  78  122 166 34  78  122 166'
    y_positions = '34  34  34  34  78  78  78  78 '
    z_positions = '0   0   0   0   0   0   0   0  '
    execute_on = 'initial'
    threshold = 0.2
    connecting_threshold = 0.08
    int_width = 8
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalColoringIC]
      polycrystal_ic_uo = circle_IC
    [../]
  [../]
[]

[Kernels]
  [./dt_gr0]
    type = TimeDerivative
    variable = gr0
  [../]
  [./dt_gr1]
    type = TimeDerivative
    variable = gr1
  [../]
  [./dt_gr2]
    type = TimeDerivative
    variable = gr2
  [../]
  [./dt_gr3]
    type = TimeDerivative
    variable = gr3
  [../]
  [./dt_gr4]
    type = TimeDerivative
    variable = gr4
  [../]
  [./dt_gr5]
    type = TimeDerivative
    variable = gr5
  [../]
[]

[AuxKernels]
  [./bnds_aux]
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  num_steps = 0
[]

[Outputs]
  exodus = true
  csv = false
[]
