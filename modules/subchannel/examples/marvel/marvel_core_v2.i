################################################################################
## Marvel surrogate SCM model                                                 ##
## Pronghorn Subchannel simulation                                            ##
## POC : Vasileios Kyriakopoulos, vasileios.kyriakopoulos@inl.gov             ##
################################################################################
# Parisi, Carlo, and Y. Arafat. "MARVEL Thermal-hydraulics: Normal and Accidental Conditions."
# Transactions of the American Nuclear Society 127.1 (2022): 1090-1093
# T_in = 738.15 #K
# mass_flow = 1.53 #kg/sec
# flow_area = 0.00891407 #m2
# mass_flux_in = '${fparse mass_flow/flow_area}' # [kg/sm2]
# P_out = 303975 # Pa
power = 85e3 # W
T_in = 6.591941e+02 #K
mass_flux_in = 11
P_out = 3.996523e+05
heated_length = 0.51
unheated_length_entry = 0.2
unheated_length_exit = 0.2
total_length = '${fparse heated_length + unheated_length_entry + unheated_length_exit}'
[TriSubChannelMesh]
    [subchannel]
        type = TriSubChannelMeshGenerator
        nrings = 4
        n_cells = 60
        flat_to_flat = 0.22 #0.21667
        heated_length = ${heated_length}
        unheated_length_entry = ${unheated_length_entry}
        unheated_length_exit = ${unheated_length_exit}
        pin_diameter = 0.03269
        pitch = 0.0346514
        dwire = 0.0
        hwire = 0.0
        spacer_z = '0.0'
        spacer_k = '0.0'
    []

    [fuel_pins]
        type = TriPinMeshGenerator
        input = subchannel
        nrings = 4
        n_cells = 60
        heated_length = ${heated_length}
        unheated_length_entry = ${unheated_length_entry}
        unheated_length_exit = ${unheated_length_exit}
        pitch = 0.0346514
    []
[]

[AuxVariables]
    [mdot]
        block = subchannel
    []
    [SumWij]
        block = subchannel
    []
    [P]
        block = subchannel
    []
    [DP]
        block = subchannel
    []
    [h]
        block = subchannel
    []
    [T]
        block = subchannel
    []
    [rho]
        block = subchannel
    []
    [mu]
        block = subchannel
    []
    [S]
        block = subchannel
    []
    [w_perim]
        block = subchannel
    []
    [displacement]
        block = subchannel
    []
    [q_prime]
        block = fuel_pins
    []
    [Tpin]
        block = fuel_pins
    []
    [Dpin]
        block = fuel_pins
    []
[]

[FluidProperties]
    [NaK]
        type = NaKFluidProperties
        weight_fraction_K = 0.778
    []
[]

[Problem]
    type = TriSubChannel1PhaseProblem
    fp = NaK
    n_blocks = 1
    P_out = report_pressure_outlet
    CT = 2.6
    compute_density = true
    compute_viscosity = true
    compute_power = true
    P_tol = 1.0e-4
    T_tol = 1.0e-4
    implicit = true
    segregated = false
    staggered_pressure = false
    monolithic_thermal = false
    verbose_multiapps = false
    verbose_subchannel = true
    deformation = false
[]

[ICs]
    [S_IC]
        type = MarvelTriFlowAreaIC
        variable = S
    []

    [w_perim_IC]
        type = MarvelTriWettedPerimIC
        variable = w_perim
    []

    [q_prime_IC]
        type = SCMTriPowerIC
        variable = q_prime
        power = ${power} #W
        filename = "pin_power_profile37.txt"
    []

    [T_ic]
        type = ConstantIC
        variable = T
        value = ${T_in}
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
        fp = NaK
    []

    [rho_ic]
        type = RhoFromPressureTemperatureIC
        variable = rho
        p = ${P_out}
        T = T
        fp = NaK
    []

    [h_ic]
        type = SpecificEnthalpyFromPressureTemperatureIC
        variable = h
        p = ${P_out}
        T = T
        fp = NaK
    []

    [mdot_ic]
        type = SCMMassFlowRateIC
        variable = mdot
        area = S
        mass_flux = 0.0
    []

    [Dpin_IC]
        type = ConstantIC
        variable = Dpin
        value = 0.03269
    []
[]

[AuxKernels]
    [T_in_bc]
        type = PostprocessorConstantAux
        variable = T
        boundary = inlet
        postprocessor = report_temperature_inlet
        execute_on = 'timestep_begin'
        block = subchannel
    []
    [mdot_in_bc]
        type = SCMMassFlowRateAux
        variable = mdot
        boundary = inlet
        area = S
        mass_flux = report_mass_flux_inlet
        execute_on = 'timestep_begin'
        block = subchannel
    []
[]

[Outputs]
    exodus = true
    csv = true
[]

[Executioner]
    type = Steady
[]

[Postprocessors]
    [total_pressure_drop_SC]
        type = SubChannelDelta
        variable = P
        execute_on = 'timestep_end'
    []

    [Pressure_Gradient_SC]
        type = ParsedPostprocessor
        pp_names = 'total_pressure_drop_SC'
        expression = 'total_pressure_drop_SC/${total_length}'
        execute_on = 'timestep_end'
    []

    [Total_power]
        type = ElementIntegralVariablePostprocessor
        variable = q_prime
        block = fuel_pins
    []

    [report_mass_flux_inlet]
        type = Receiver
        default = ${mass_flux_in}
    []

    [report_temperature_inlet]
        type = Receiver
        default = ${T_in}
    []

    [report_pressure_outlet]
        type = Receiver
        default = ${P_out}
    []
[]

################################################################################
#### A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
    [viz]
        type = FullSolveMultiApp
        input_files = "marvel_core_v2_viz.i"
        execute_on = "FINAL"
    []
[]

[Transfers]
    [subchannel_transfer]
        type = SCMSolutionTransfer
        to_multi_app = viz
        variable = 'mdot SumWij P DP h T rho mu S displacement w_perim'
    []
    [pin_transfer]
        type = SCMPinSolutionTransfer
        to_multi_app = viz
        variable = 'Dpin Tpin q_prime'
    []
[]
