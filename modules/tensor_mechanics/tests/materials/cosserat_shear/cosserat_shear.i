[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 2
  zmax = 0.2
[]

[MeshModifiers]
  [./bottom_center]
    type = AddExtraNodeset
    boundary = 101
    coord = '0.5 0 0.1'
  [../]
  [./bottom_right_middle]
    type = AddExtraNodeset
    boundary = 102
    coord = '1 0 0.1'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./wc_x]
  [../]
  [./wc_y]
  [../]
  [./wc_z]
  [../]
[]

[Kernels]
  [./x_elastic]
    type = StressDivergenceTensors
    variable = disp_x
    disp_z = disp_z
    disp_y = disp_y
    disp_x = disp_x
    component = 0
  [../]
  [./y_elastic]
    type = StressDivergenceTensors
    variable = disp_y
    disp_z = disp_z
    disp_y = disp_y
    disp_x = disp_x
    component = 1
  [../]
  [./z_elastic]
    type = StressDivergenceTensors
    variable = disp_z
    disp_z = disp_z
    disp_y = disp_y
    disp_x = disp_x
    component = 2
  [../]
  [./x_couple]
    type = StressDivergenceTensors
    variable = wc_x
    disp_z = wc_z
    disp_y = wc_y
    disp_x = wc_x
    component = 0
    appended_property_name = _couple
  [../]
  [./y_couple]
    type = StressDivergenceTensors
    variable = wc_y
    disp_z = wc_z
    disp_y = wc_y
    disp_x = wc_x
    component = 1
    appended_property_name = _couple
  [../]
  [./z_couple]
    type = StressDivergenceTensors
    variable = wc_z
    disp_z = wc_z
    disp_y = wc_y
    disp_x = wc_x
    component = 2
    appended_property_name = _couple
  [../]
  [./x_moment]
    type = MomentBalancing
    variable = wc_x
    disp_z = wc_z
    disp_y = wc_y
    disp_x = wc_x
    component = 0
  [../]
  [./y_moment]
    type = MomentBalancing
    variable = wc_y
    disp_z = wc_z
    disp_y = wc_y
    disp_x = wc_x
    component = 1
  [../]
  [./z_moment]
    type = MomentBalancing
    variable = wc_z
    disp_z = wc_z
    disp_y = wc_y
    disp_x = wc_x
    component = 2
  [../]
[]

[BCs]
  [./x_bottom]
    type = DirichletBC
    variable = disp_x
    boundary = 101
    value = 0
  [../]
  [./y_bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./z_bottom]
    type = DirichletBC
    variable = disp_z
    boundary = '101 102'
    value = 0
  [../]
  [./wc_x_bottom]
    type = DirichletBC
    variable = wc_x
    boundary = bottom
    value = 0
  [../]
  [./wc_y_bottom]
    type = DirichletBC
    variable = wc_y
    boundary = bottom
    value = 0
  [../]
  [./wc_z_bottom]
    type = DirichletBC
    variable = wc_z
    boundary = bottom
    value = 0
  [../]
  [./top_force]
    type = NeumannBC
    variable = disp_y
    boundary = top
    value = 1
  [../]
[]

[Materials]
  [./cosserat]
    type = CosseratLinearElasticMaterial
    block = 0
    disp_z = disp_z
    disp_y = disp_y
    disp_x = disp_x
    B_ijkl = 0.5
    C_ijkl = '5 1 1 5 1 5 2 2 2'
    wc_z = wc_z
    wc_y = wc_y
    wc_x = wc_x
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
  console = true
[]

