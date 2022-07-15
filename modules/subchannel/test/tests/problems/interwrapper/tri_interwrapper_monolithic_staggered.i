T_in = 360.0
P_out = 4.923e6 # Pa


[TriInterWrapperMesh]
  [sub_channel]
    type = TriInterWrapperMeshGenerator
    nrings = 3
    n_cells = 10
    flat_to_flat = 0.16
    heated_length = 1.0
    assembly_pitch = 0.17
    side_bypass = 0.01
  []
[]


[AuxVariables]
    [mdot]
    []
    [SumWij]
    []
    [P]
    []
    [DP]
    []
    [h]
    []
    [T]
    []
    [rho]
    []
    [mu]
    []
    [S]
    []
    [w_perim]
    []
    [q_prime]
    []
[]


[Modules]
    [FluidProperties]
        [sodium]
            type = PBSodiumFluidProperties
        []
    []
[]


[ICs]
    [S_IC]
        type = TriInterWrapperFlowAreaIC
        variable = S
    []

    [w_perim_IC]
        type = TriInterWrapperWettedPerimIC
        variable = w_perim
    []

    [q_prime_IC]
        type = TriInterWrapperPowerIC
        variable = q_prime
    []

    [T_ic]
        type = ConstantIC
        variable = T
        value = ${T_in}
    []

    [P_ic]
        type = ConstantIC
        variable = P
        value = 2.0e5
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
        fp = sodium
    []

    [rho_ic]
        type = RhoFromPressureTemperatureIC
        variable = rho
        p = ${P_out}
        T = T
        fp = sodium
    []

    [h_ic]
        type = SpecificEnthalpyFromPressureTemperatureIC
        variable = h
        p = ${P_out}
        T = T
        fp = sodium
    []

    [mdot_ic]
        type = ConstantIC
        variable = mdot
        value = 0.1
    []
[]


[SubChannel]
    type = LiquidMetalInterWrapper1PhaseProblem
    fp = sodium
    n_blocks = 1
    beta = 0.1
    P_out = 2.0e5
    CT = 1.0
    compute_density = false
    compute_viscosity = false
    compute_power = false
    P_tol = 1.0e-6
    T_tol = 1.0e-6
    implicit = true
    segregated = false
    staggered_pressure = true
    monolithic_thermal = false
    interpolation_scheme = 'central_difference'
[]


[AuxKernels]
[]


[Outputs]
  exodus = true
  checkpoint = false
[]


[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]
