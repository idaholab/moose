[Mesh]
  file=Grain4.msh
[]

[GlobalParams]
  displacements= 'disp_x disp_y'
[]

[UserObjects]
  [./euler_angle_file]
    type = EulerAngleFileReader
    file_name = Grain4.ori
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        strain = SMALL
        add_variables = true
        generate_output = 'stress_xx strain_xx stress_yy strain_yy stress_xy strain_xy vonmises_stress'
      [../]
    [../]
  [../]
[]

[Materials]
  [./ElasticityTensor]
    type = ComputeBlockRotatedElasticityTensor
    euler_angle_provider = euler_angle_file
    fill_method = symmetric9
    C_ijkl = '1.27 0.708 0.708 1.27 0.708 1.27 0.7355 0.7355 0.7355'
    # C_ijkl = '1.27e5 0.708e5 0.708e5 1.27e5 0.708e5 1.27e5 0.7355e5 0.7355e5 0.7355e5'
    offset = 1
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  start_time = 0.0
  dt=1
  num_steps = 25
[]

[Outputs]
  exodus = true
[]
