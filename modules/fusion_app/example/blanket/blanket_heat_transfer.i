BZ = 'OB_BZ_0 OB_BZ_1 OB_BZ_2 OB_BZ_3 OB_BZ_4 OB_BZ_5 OB_BZ_6 OB_BZ_7 OB_BZ_8'

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'create_bdries_from_blocks_in.e'
  []

#  parallel_type = distributed
#  partitioner = parmetis
  skip_refine_when_use_split = false
  skip_deletion_repartition_after_refine = true
[]

[Outputs]
#  exodus = true
  nemesis = true
[]

#[Problem]
#  solve = false
#[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  line_search = 'none'
  l_tol = 1e-3
  l_max_its = 20
  nl_rel_tol = 1e-6
  automatic_scaling = true
  fixed_point_max_its = 100
  fixed_point_rel_tol = 1e-6
  [./Quadrature]
    allow_negative_qweights = false
  [../]
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
  [heat_generation]
    type = CoupledForce
    variable = temp
    block = 'OB_radial_plate OB_shell
             ${BZ} OB_FW_SW OB_Armor'
    v = power_density
  []
[]

[BCs]
#  [heat_channel]
#    type = DirichletBC
#    variable = temp
#    boundary = "${shell_channel}"
#    value = 685.65 # K
#  []

  [FW_BC]
    type = NeumannBC
    variable = temp
    boundary = 'armor'
    value = 250000 # 0.25 MW/m^2
  []

  [channel]
    type = CoupledConvectiveHeatFluxBC
    variable = temp
    htc = htc
    T_infinity = Tfluid
    boundary = '${FW_channel} ${shell_channel} ${plate_channel}'
#    boundary = '${FW_channel} ${plate_channel}'
  []
[]

[Materials]
  [breeder_material_BZ_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = breeder
    temp = temp
    block = "OB_BZ_1 OB_BZ_3 OB_BZ_5 OB_BZ_6 OB_BZ_7 OB_BZ_8"
  []
  [multiplier_material_BZ_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = multiplier
    temp = temp
    block = "OB_BZ_0 OB_BZ_2 OB_BZ_4"
  []
  [breeder_material_plate_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = F82H
    temp = temp
    block = "OB_radial_plate OB_shell OB_FW_SW"
  []
  [armor_material_conductivity]
    type = HeatConductionMaterial
#    thermal_conductivity = 150.0 # W/m-K  at 400C
    thermal_conductivity_temperature_function = tungsten
    temp = temp
    block = "OB_Armor"
  []
[]

[AuxVariables]
  [power_density]
    order = CONSTANT
    family = MONOMIAL
  []
  [Tfluid]
  []
  [htc]
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

[Postprocessors]
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = power_density
    block = 'OB_radial_plate OB_shell
             ${BZ} OB_FW_SW OB_Armor'
  []
  [FW_flux]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 'armor'
    diffusivity = 'thermal_conductivity'
  []
  [total_power]
    type = LinearCombinationPostprocessor
    pp_names = 'power FW_flux'
    pp_coefs = '1. -1.'
  []
  [volume]
    type = VolumePostprocessor
    block = '${BZ}'
  []
  [channel_flux]
    type = LinearCombinationPostprocessor
    pp_names = 'shell_channel_flux plate_channel_flux FW_channel_flux'
    pp_coefs = '1. 1. 1.'
  []
  [shell_channel_flux]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = '${shell_channel}'
    diffusivity = 'thermal_conductivity'
  []
  [plate_channel_flux]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = '${plate_channel}'
    diffusivity = 'thermal_conductivity'
  []
  [FW_channel_flux]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = '${FW_channel}'
    diffusivity = 'thermal_conductivity'
  []
[]

[AuxKernels]
  [power]
    type = FNSFSourceAux
    variable = power_density
    block = 'OB_radial_plate OB_shell
             ${BZ} OB_FW_SW OB_Armor'
    inner_xi = '-67.1621 -45.0782 -26.682 -8.91958 8.91958 26.682 45.0782 67.1621'
    outer_xi = '-80.7101 -53.2501 -29.8105 -9.58303 9.58303 29.8105 53.2501 80.7101'
    depth = '0.1 0.102 0.14 0.163 0.181 0.199 0.217 0.277 0.295 0.321 0.339 0.443 0.461 0.531 0.549 0.652 0.67 0.82 0.838 1.08 1.1'
    source = '2.7544e+07 2.7544e+07 2.7544e+07 2.7544e+07 2.7544e+07 2.7544e+07 2.7544e+07 4.6228e+06 4.6228e+06 4.6228e+06 4.6228e+06 4.6228e+06 4.6228e+06 4.6228e+06 6.4374e+06 6.4374e+06 6.4374e+06 6.4374e+06 6.4374e+06 6.4374e+06 6.4374e+06 6.3422e+06 6.3422e+06 6.3422e+06 6.3422e+06 6.3422e+06 6.3422e+06 6.3422e+06 1.6260e+07 1.6260e+07 1.6260e+07 1.6260e+07 1.6260e+07 1.6260e+07 1.6260e+07 4.2483e+06 4.2483e+06 4.2483e+06 4.2483e+06 4.2483e+06 4.2483e+06 4.2483e+06 2.7470e+06 2.7470e+06 2.7470e+06 2.7470e+06 2.7470e+06 2.7470e+06 2.7470e+06 2.6035e+06 2.6035e+06 2.6035e+06 2.6035e+06 2.6035e+06 2.6035e+06 2.6035e+06 8.3437e+06 8.3437e+06 8.3437e+06 8.3437e+06 8.3437e+06 8.3437e+06 8.3437e+06 1.5133e+06 1.5133e+06 1.5133e+06 1.5133e+06 1.5133e+06 1.5133e+06 1.5133e+06 9.5635e+05 9.5635e+05 9.5635e+05 9.5635e+05 9.5635e+05 9.5635e+05 9.5635e+05 7.4320e+05 7.4320e+05 7.4320e+05 7.4320e+05 7.4320e+05 7.4320e+05 7.4320e+05 1.0826e+06 1.0826e+06 1.0826e+06 1.0826e+06 1.0826e+06 1.0826e+06 1.0826e+06 1.3815e+05 1.3815e+05 1.3815e+05 1.3815e+05 1.3815e+05 1.3815e+05 1.3815e+05 2.7832e+05 2.7832e+05 2.7832e+05 2.7832e+05 2.7832e+05 2.7832e+05 2.7832e+05 5.2306e+04 5.2306e+04 5.2306e+04 5.2306e+04 5.2306e+04 5.2306e+04 5.2306e+04 1.2672e+05 1.2672e+05 1.2672e+05 1.2672e+05 1.2672e+05 1.2672e+05 1.2672e+05 2.8766e+04 2.8766e+04 2.8766e+04 2.8766e+04 2.8766e+04 2.8766e+04 2.8766e+04 1.0178e+05 1.0178e+05 1.0178e+05 1.0178e+05 1.0178e+05 1.0178e+05 1.0178e+05 2.7972e+04 2.7972e+04 2.7972e+04 2.7972e+04 2.7972e+04 2.7972e+04 2.7972e+04'
    []
[]

[MultiApps]
  [channel]
    type = FullSolveMultiApp
    input_files = 'channel_fw.i channel_plate.i channel_shell.i'
    positions_file = 'positions_fw.txt positions_plate.txt positions_shell.txt'
    cli_args_files = 'cliargs_fw.txt cliargs_plate.txt cliargs_shell.txt'
#    input_files = 'channel_fw.i channel_plate.i '
#    positions_file = 'positions_fw.txt positions_plate.txt'
#    cli_args_files = 'cliargs_fw.txt cliargs_plate.txt'
    max_procs_per_app = 1
    keep_solution_during_restore = true
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
