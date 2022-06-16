[Mesh]
  type = FileMesh
  file = simple_pb.e
[]

[Variables]
  [./temp_wall]
    block = 'left right'
  [../]
  [./temp_fluid]
    block = 'center'
  [../]
[]

[Kernels]
  [./wall_conduction]
    type = ADHeatConduction
    variable = temp_wall
  [../]
  [./heat_source]
    type = HeatSource
    value = 1e3    # W/m^3
    variable = temp_fluid
    block = 'center'
  [../]
  [./center_conduction]
    type = ADHeatConduction
    variable = temp_fluid
    block = 'center'
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = temp_wall
    boundary = 'right'
    value = 300
  [../]
  [./left]
    type = DirichletBC
    variable = temp_wall
    boundary = 'left'
    value = 100
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]

[Materials]
  [./walls]
    type = ADHeatConductionMaterial
    thermal_conductivity = 10    # W/m k
    block = 'left right'
    specific_heat = .49e3    # J/kg k
  [../]
  [./pb]
    type = ADHeatConductionMaterial
    thermal_conductivity = 1
    specific_heat = .49e3    # J/kg K
    block = 'center'
  [../]
  [./alpha_wall]
    type = ADGenericConstantMaterial
    prop_names = 'alpha_wall'
    prop_values = '1'
    block = 'center'
  [../]
[]

[InterfaceKernels]
  [./left_center_wrt_center]
    type = ConjugateHeatTransfer
    variable = temp_fluid
    T_fluid = temp_fluid
    neighbor_var = 'temp_wall'
    boundary = 'left_center_wrt_center'
    htc = 'alpha_wall'
  [../]
  [./right_center_wrt_center]
    type = ConjugateHeatTransfer
    variable = temp_fluid
    T_fluid = temp_fluid
    neighbor_var = 'temp_wall'
    boundary = 'right_center_wrt_center'
    htc = 'alpha_wall'
  [../]
[]

[Preconditioning]
  [./Hypre]
    type = SMP
    petsc_options_value = 'lu hypre'
    full = true
    petsc_options_iname = '-pc_type -pc_hypre_type'
  [../]
[]
