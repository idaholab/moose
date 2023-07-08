Channels = 'CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10 CH11 CH12 CH13 CH14 CH15 CH16 CH17 CH18 CH19 CH20 CH21 CH22 CH23 CH24 CH25 CH26'

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'Meshes/Blanket_Simple.msh'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

[VectorPostprocessors]
  [temp_y]
    type = LineValueSampler
    start_point = '-0.01 0 0.0183'
    end_point = '-0.01 0.119 0.0183'
    num_points = 100
    variable = temp
    sort_by = y
  []
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
  [temp]
    initial_condition = 800 # K
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temp
  []
  [pd_shield]
    type = HeatSource
    variable = temp
    block = 'Shield'
    value = 5.5088e6 #2.7544e07 original 
  []
  [pd_fw]
    type = HeatSource
    variable = temp
    value = 9.2456e5 #4.6228e06 original
    block = 'First_Wall'
  []
  [pd_mult]
    type = HeatSource
    variable = temp
    value = 1.28748e6 #6.4374e06 original
    block = 'Multiplier'
  []
  [pd_ts]
    type = HeatSource
    variable = temp
    value = 1.26844e6 #6.3422e06 original
    block = 'Toroidal_Plate1'
  []
  [pd_ts1]
    type = HeatSource
    variable = temp
    value = 3.252e6 #1.6260e07 original
    block = 'Breeder'
  []
  [pd_ts2]
    type = HeatSource
    variable = temp
    value = 8.4966e5 #4.2483e6 original 
    block = 'Toroidal_Plate2'
  []
[]

[BCs]
  [FW_BC]
    type = NeumannBC
    variable = temp
    boundary = 'Heated_Surface'
    value = 3202.5 # 0.25 MW/m^2 3202.5 for simple blanket mesh
  []

  [channel]
    type = CoupledConvectiveHeatFluxBC
    variable = temp
    htc = htc
    T_infinity = Tfluid
    boundary = ${Channels}
  []
[]

[Materials]
  [breeder_material_BZ_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = breeder
    temp = temp
    block = 'Breeder'
  []
  [multiplier_material_BZ_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = multiplier
    temp = temp
    block ='Multiplier'
  []
  [breeder_material_plate_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = F82H
    temp = temp
    block = 'First_Wall Toroidal_Plate1 Toroidal_Plate2'
  []
  [armor_material_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = tungsten
    temp = temp
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

[AuxVariables]
  [Tfluid]
  []
  [htc]
  []
[]

[MultiApps]
  [channel]
    type = FullSolveMultiApp
    input_files = 'Blanket_Slice_CH1.i
                  Blanket_Slice_CH2.i
                  Blanket_Slice_CH3.i
                  Blanket_Slice_CH4.i
                  Blanket_Slice_CH5.i
                  Blanket_Slice_CH6.i
                  Blanket_Slice_CH7.i
                  Blanket_Slice_CH8.i
                  Blanket_Slice_CH9.i
                  Blanket_Slice_CH10.i
                  Blanket_Slice_CH11.i
                  Blanket_Slice_CH12.i
                  Blanket_Slice_CH13.i
                  Blanket_Slice_CH14.i
                  Blanket_Slice_CH15.i
                  Blanket_Slice_CH16.i
                  Blanket_Slice_CH17.i
                  Blanket_Slice_CH18.i
                  Blanket_Slice_CH19.i
                  Blanket_Slice_CH20.i
                  Blanket_Slice_CH21.i
                  Blanket_Slice_CH22.i
                  Blanket_Slice_CH23.i
                  Blanket_Slice_CH24.i
                  Blanket_Slice_CH25.i
                  Blanket_Slice_CH26.i'
    positions = '0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0
                0 0 0'
    cli_args_files = 'clig_args.txt'
  []
[]

[Transfers]
  [wall_temperature]
    type = MultiAppMapNearestNodeTransfer
    to_multi_app = channel
    source_variable = temp
    variable = T_wall
  []

  [fluid_temperature]
    type = MultiAppMapNearestNodeTransfer
    from_multi_app = channel
    variable = Tfluid
    source_variable = T
  []

  [htc]
    type = MultiAppMapNearestNodeTransfer
    from_multi_app = channel
    variable = htc
    source_variable = Hw
  []
[]
