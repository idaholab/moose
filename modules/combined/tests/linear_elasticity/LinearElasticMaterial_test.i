# This input file is designed to test the LinearElasticMaterial class. 

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./diffused]
    order = FIRST
    family = LAGRANGE
     [./InitialCondition]
      type = RandomIC
     [../]
  [../]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[TensorMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
 [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[] 

[Materials]
  [./Anisotropic]
    type = LinearElasticMaterial
    block = 0
    disp_x = disp_x
    disp_y = disp_y
 
    #set from elk/tests/anisotropic_path/anisotropic_patch_test.i 
    all_21 = false
    #reading C_11  C_12  C_13  C_22  C_23  C_33  C_44  C_55  C_66
    C_ijkl ='1.0e6  0.0   0.0 1.0e6  0.0  1.0e6 0.5e6 0.5e6 0.5e6'	
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = diffused
    boundary = '1'
    value = 1
  [../]

  [./top]
    type = DirichletBC
    variable = diffused
    boundary = '2'
    value = 0
  [../]

  [./disp_x_BC]
    type = DirichletBC
    variable = disp_x
    boundary = '0 2'
    value = 0.5
  [../]

  [./disp_x_BC2]
    type = DirichletBC
    variable = disp_x
    boundary = '1 3'
    value = 0.01
  [../]

  [./disp_y_BC]
    type = DirichletBC
    variable = disp_y
    boundary = '0 2'
    value = 0.8
  [../]

  [./disp_y_BC2]
    type = DirichletBC
    variable = disp_y
    boundary = '1 3'
    value = 0.02
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = LinearElasticMaterial
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
