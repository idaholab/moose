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

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./diffused]
     [./InitialCondition]
      type = RandomIC
     [../]
  [../]
[]

[Physics/SolidMechanics/QuasiStatic/All]
  strain = SMALL
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    #reading C_11  C_12  C_13  C_22  C_23  C_33  C_44  C_55  C_66
    C_ijkl ='1.0e6  0.0   0.0 1.0e6  0.0  1.0e6 0.5e6 0.5e6 0.5e6'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
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
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
