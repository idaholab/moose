######## BC's #################
T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 1e+6 * 17.00 / 3600.}'
P_out = 4.923e6 # Pa
heated_length = 1.0
######## Geometry #
[GlobalParams]
    nx = 2
    ny = 2
    n_cells = 20
    pitch = 0.0126
    pin_diameter = 0.00950
    side_gap = 0.00095
    heated_length = ${heated_length}
    spacer_z = '0.0'
    spacer_k = '0.0'
    power = 10000.0 # W
[]

[QuadSubChannelMesh]
    [subchannel]
        type = SCMQuadSubChannelMeshGenerator
    []

    [fuel_pins]
        type = SCMQuadPinMeshGenerator
        input = subchannel
    []
[]

[Functions]
    [axial_heat_rate]
        type = ParsedFunction
        expression = '(pi/2)*sin(pi*z/L)'
        symbol_names = 'L'
        symbol_values = '${heated_length}'
    []
[]

[FluidProperties]
    [water]
        type = Water97FluidProperties
    []
[]

[SubChannel]
    type = QuadSubChannel1PhaseProblem
    fp = water
    n_blocks = 1
    compute_density = true
    compute_viscosity = true
    compute_power = true
    P_out = ${P_out}
    verbose_subchannel = true
    friction_closure = 'MATRA'
    pin_HTC_closure = 'Dittus-Boelter'
    mixing_closure ='constant_beta'
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
  [Dittus-Boelter]
    type = SCMHTCDittusBoelter
  []
  [constant_beta]
    type = SCMMixingConstantBeta
    beta = 0.006
    CT = 2.6
  []
[]

[ICs]
    [S_IC]
        type = SCMQuadFlowAreaIC
        variable = S
    []

    [w_perim_IC]
        type = SCMQuadWettedPerimIC
        variable = w_perim
    []

    [q_prime_IC]
        type = SCMQuadPowerIC
        variable = q_prime
        filename = "power_profile.txt"
        axial_heat_rate = axial_heat_rate
    []

    [T_ic]
        type = ConstantIC
        variable = T
        value = ${T_in}
    []

    [Dpin_ic]
        type = ConstantIC
        variable = Dpin
        value = 0.00950
    []

    [P_ic]
        type = ConstantIC
        variable = P
        value = 0.0
    []

    [DP_ic]
        type = ConstantIC
        variable = DP
        value = 0.0
    []

    [Viscosity_ic]
        type = ViscosityIC
        variable = mu
        p = ${P_out}
        T = T
        fp = water
    []

    [rho_ic]
        type = RhoFromPressureTemperatureIC
        variable = rho
        p = ${P_out}
        T = T
        fp = water
    []

    [h_ic]
        type = SpecificEnthalpyFromPressureTemperatureIC
        variable = h
        p = ${P_out}
        T = T
        fp = water
    []

    [mdot_ic]
        type = ConstantIC
        variable = mdot
        value = 0.0
    []
[]

[AuxKernels]
    [T_in_bc]
        type = ConstantAux
        variable = T
        boundary = inlet
        value = ${T_in}
        execute_on = 'timestep_begin'
    []
    [mdot_in_bc]
        type = SCMMassFlowRateAux
        variable = mdot
        boundary = inlet
        area = S
        mass_flux = ${mass_flux_in}
        execute_on = 'timestep_begin'
    []
[]

[Postprocessors]
    [DP]
        type = SubChannelDelta
        variable = P
        execute_on = 'timestep_end'
    []
[]

[Outputs]
    csv = true
[]

[Executioner]
    type = Steady
[]
