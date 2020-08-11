[Mesh]
  type = GeneratedMesh
  dim  = 1
  nx   = 20
  xmax = 1000
  ny   = 2
  ymax = 50
[]

[UserObjects]
  [./neuron_test]
    type = NeuralNetwork
    weights_file = "NN_struct.XML"
  [../]
[]

[Variables]

  [./eta]
  [../]
[]

[AuxVariables]
  #COMPONENT #1
  [./c_Ni]
  [../]
  [./c_Ni_metal]
  [../]
  [./c_Ni_melt]
  [../]
  [./w_Ni]
  [../]

  #COMPONENT #2
  [./c_Cr]
  [../]
  [./c_Cr_metal]
  [../]
  [./c_Cr_melt]
  [../]
  [./w_Cr]
  [../]

[]

[Kernels]
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]
[]

[ICs]
  [./c_ni_metal_initial]
    type = NeuralNetworkIC
    variable = c_Ni_metal
    InputVariables = 'c_Ni c_Cr eta'
    NeuralNetwork_user_object = neuron_test
    op_id = 0
  [../]
  [./c_ni_melt_initial]
    type = NeuralNetworkIC
    variable = c_Ni_melt
    InputVariables = 'c_Ni c_Cr eta'
    NeuralNetwork_user_object = neuron_test
    op_id = 1
  [../]
  [./c_Cr_metal_initial]
    type = NeuralNetworkIC
    variable = c_Cr_metal
    InputVariables = 'c_Ni c_Cr eta'
    NeuralNetwork_user_object = neuron_test
    op_id = 2
  [../]
  [./c_cr_melt_initial]
    type = NeuralNetworkIC
    variable = c_Cr_melt
    InputVariables = 'c_Ni c_Cr eta'
    NeuralNetwork_user_object = neuron_test
    op_id = 3
  [../]
  [./eta_metal_inital]
    type = FunctionIC
    variable = 'eta'
    function = 'if(x>500,0.0,1.0)'
  [../]
  [./c_global_inital]
    type = FunctionIC
    variable = 'c_Ni'
    function = 'if(x>500,0.02,0.8)'
  [../]
  [./c_Cr_global_inital]
    type = FunctionIC
    variable = 'c_Cr'
    function = 'if(x>500,0.19,0.003)'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  scheme = bdf2
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  num_steps = 0
[]
[Postprocessors]
  [./elapsed]
    type = PerfGraphData
    section_name = "Root"
    data_type = total
  [../]
[]

[Outputs]
  exodus = true
  file_base = 'neural_net_ic_test'
[]
