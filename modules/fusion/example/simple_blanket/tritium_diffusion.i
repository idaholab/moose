[Mesh]
    [fmg]
      type = FileMeshGenerator
      file = 'Meshes/Blanket_OneRow.msh'
    []
[]
  
[Outputs]
    exodus = true
[]
  
[Preconditioning]
    [smp]
      type = SMP
      full = true
    []
[]
  
[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
[]
  
[Variables]
    [tritium]
      initial_condition = 0
    []
[]
  
[Kernels]
    [diffusion]
      type = ADDiffusion
      variable = tritium
    []
    [pd_breeder]
      type = BodyForce
      variable = tritium
      value = 1.681e-5
      block = 'Breeder'
    []
[]
  
[BCs] 
  [FW_BC]
    type = NeumannBC
    variable = tritium
    boundary = 'Heated_Surface'
    value = 0
  []

  [BW_BC]
    type = NeumannBC
    variable = tritium
    boundary = 'Back_Wall'
    value = 0
  []

  [CH_BC]
    type = NeumannBC
    variable = tritium
    boundary = 'CH1 CH2'
    value = 0
  []
[]
  
[AuxVariables]
    [temperature]
    []
[]

[Materials]
  [breeder_material_BZ_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = breeder
    temp = temperature
    block = 'Breeder'
  []
  [multiplier_material_BZ_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = multiplier
    temp = temperature
    block ='Multiplier'
  []
  [breeder_material_plate_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = F82H
    temp = temperature
    block = 'First_Wall Toroidal_Plate'
  []
  [armor_material_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = tungsten
    temp = temperature
    block = 'Shield'
  []
[]

[Functions]
  [./F82H]
    # Multiphyics modeling of the FW/Blanket of the US fusion nuclear science facility
    # Y. Huang,
    # Fig. 7
    type = PiecewiseLinear
    x = '293.93 390.03 491.33 592.63 696.53 779.64 865.36 943.28 1008.21 1067.96 1140.68' # K
    y = '24.46 23.04 21.33 19.24 16.77 14.49 11.93 9.37 6.80 4.34 1.30' # W/mK
  [../]

  [./breeder]
    # A Novel Cellular Breeding Material For Transformative Solid Breeder Blankets
    # S. Sharafat
    # Figure 18
    type = PiecewiseLinear
    x = '273.23 307.47 362.71 461.40 562.54 666.43 767.05 867.29 967.50 1070.53 1170.73 1268.15 1381.42 1480.19' # K
    y = '3.96 3.66 3.43 2.86 2.49 2.20 2.00 1.92 1.85 1.85 1.78 1.64 1.26 0.66' # W/mK
  [../]

  [./multiplier]
    # Thermal conductivity of neutron irradiated Be12Ti
    # M. Uchida
    # Fig. 6
    type = PiecewiseLinear
    x = '266.83 376.31 477.36 662.62 847.89 1049.99 1243.68' #K
    y = '12.96 18.31 19.44 22.82 25.07 32.96 50.42' # W/mK
  [../]
  [./tungsten]
    # Thermal properties of pure tungsten and its alloys for fusion applications
    # Makoto Fukuda
    # Fig. 5
    type = PiecewiseLinear
    x = '365.44 464.99 556.82 660.93 757.28 858.82 947.70 1115.15 1254.55 1343.22' #K
    y = '178.86 162.76 149.22 142.95 140.10 134.69 129.26 122.23 119.01 117.86' # W/mK
  [../] 
[]