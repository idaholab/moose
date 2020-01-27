[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [./msh]
    type = FileMeshGenerator
    file = two_blocks.e
  []
[]

[Variables]
  [./temp]
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_yz stress_xz stress_xy'
  [../]
[]


[ThermalContact]
  [./gap_conductivity]
    type = GapHeatTransfer
    variable = temp
    slave = 4
    master = 5
    gap_conductance = 1e8
    quadrature = true
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./right_y1]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 3
    function = 0
  [../]
  [./right_y2]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = 0.01*t
  [../]
  [./temp1]
    type = DirichletBC
    variable = temp
    boundary = 7
    value = 1000.0
  [../]

  [./temp2]
    type = DirichletBC
    variable = temp
    boundary = 6
    value = 500.0
  [../]
[]

[Materials]
  [./density]
    type = Density
    density = 100
  [../]
  [./temp]
    type = HeatConductionMaterial
    thermal_conductivity = 1e5
    specific_heat = 620.0
  [../]
  [./Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e8'
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

[Outputs]
  exodus = true
[]

[Executioner]
  automatic_scaling = true
  type = Transient
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  solve_type = NEWTON
  nl_max_its = 10
  l_tol = 1e-10
  l_max_its = 50
  start_time = 0.0
  dt = 0.2
  dtmin = 0.2
  num_steps = 1
  line_search = none
[]
