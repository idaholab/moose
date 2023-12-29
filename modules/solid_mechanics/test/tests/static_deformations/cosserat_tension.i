[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    zmax = 0.2
  []
  [bottom_xline1]
    type = ExtraNodesetGenerator
    new_boundary = 101
    coord = '0 0 0'
    input = generated_mesh
  []
  [bottom_xline2]
    type = ExtraNodesetGenerator
    new_boundary = 101
    coord = '0.5 0 0'
    input = bottom_xline1
  []
  [bottom_xline3]
    type = ExtraNodesetGenerator
    new_boundary = 101
    coord = '1 0 0'
    input = bottom_xline2
  []
  [bottom_zline1]
    type = ExtraNodesetGenerator
    new_boundary = 102
    coord = '0 0 0.0'
    input = bottom_xline3
  []
  [bottom_zline2]
    type = ExtraNodesetGenerator
    new_boundary = 102
    coord = '0 0 0.1'
    input = bottom_zline1
  []
  [bottom_zline3]
    type = ExtraNodesetGenerator
    new_boundary = 102
    coord = '0 0 0.2'
    input = bottom_zline2
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  Cosserat_rotations = 'wc_x wc_y wc_z'
[]

[Postprocessors]
  [./disp_y_top]
    type = PointValue
    point = '0.5 1 0.1'
    variable = disp_y
  [../]
  [./wc_z_top]
    type = PointValue
    point = '0.5 1 0.1'
    variable = wc_z
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
  [./cx_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./cy_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
  [./cz_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_z
    component = 2
  [../]
  [./x_couple]
    type = StressDivergenceTensors
    variable = wc_x
    displacements = 'wc_x wc_y wc_z'
    component = 0
    base_name = couple
  [../]
  [./y_couple]
    type = StressDivergenceTensors
    variable = wc_y
    displacements = 'wc_x wc_y wc_z'
    component = 1
    base_name = couple
  [../]
  [./z_couple]
    type = StressDivergenceTensors
    variable = wc_z
    displacements = 'wc_x wc_y wc_z'
    component = 2
    base_name = couple
  [../]
  [./x_moment]
    type = MomentBalancing
    variable = wc_x
    component = 0
  [../]
  [./y_moment]
    type = MomentBalancing
    variable = wc_y
    component = 1
  [../]
  [./z_moment]
    type = MomentBalancing
    variable = wc_z
    component = 2
  [../]
[]

[BCs]
  [./y_bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]

  [./x_line]
    type = DirichletBC
    variable = disp_z
    boundary = 101
    value = 0
  [../]
  [./z_line]
    type = DirichletBC
    variable = disp_x
    boundary = 102
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
  [./elasticity_tensor]
    type = ComputeCosseratElasticityTensor
    B_ijkl = 0.5
    E_ijkl = '1 2 1.3333'
    fill_method = 'general_isotropic'
  [../]
  [./strain]
    type = ComputeCosseratSmallStrain
  [../]
  [./stress]
    type = ComputeCosseratLinearElasticStress
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_atol -ksp_rtol'
    petsc_options_value = 'gmres bjacobi 1E-10 1E-10 10 1E-15 1E-10'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  num_steps = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = cosserat_tension_out
  exodus = true
[]
