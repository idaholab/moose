# Verification of the interfacial mass transfer closed on the transported interfacial area.
#
# A closed, uniform, quiescent box of superheated mixture. The mixture temperature starts above
# saturation, so the interfacial mass transfer generates the dispersed phase, absorbing its latent
# heat from the energy equation and driving the temperature back towards saturation. Every field
# stays spatially uniform, so the case reduces to a system of ordinary differential equations with
# an exact invariant to check against.
#
# With a constant dispersed phase density the material derivative of that density vanishes, and
# with both bubble interaction coefficients set to zero the coalescence and breakage sources
# vanish, so the only source of interfacial area is the phase change term. What remains is
#
#   rho_d d(alpha)/dt = Gamma ,        rho_d d(chi)/dt = (2/3) Gamma chi / alpha ,
#
# and dividing one by the other eliminates Gamma entirely:
#
#   d(chi)/d(alpha) = (2/3) chi / alpha   ==>   chi / alpha^(2/3) = constant .
#
# That invariant is the statement that the transfer grows the particles already present at fixed
# number density, which is the assumption the two thirds exponent encodes. It holds whatever the
# transfer rate, the temperature or the properties are, so the postprocessor below is compared
# against its initial value rather than against a computed answer. Getting the exponent, the sign
# of the coupling or the consistency between the two equations wrong all break it.
#
# The transfer rate itself is not prescribed here. It is computed by the Physics from the solved
# interfacial area, and the same computed rate is what the area equation is given, which is the
# property this case exists to exercise. The rate is also formed a second time by an explicitly
# declared WCNSFV2PInterfacialMassTransferFunctorMaterial with the same inputs, which must agree
# with the one the Physics builds exactly.
#
# The common part of the case is in interfacial-mass-transfer-base.i.

# The rate the Physics computes, declared under the name of the Physics block
gamma_name = 'mixture_interfacial_mass_transfer_rate'

# Saturation temperature of the transition. The initial superheat of 3 K drives the transfer.
T_sat = 372.0

!include interfacial-mass-transfer-base.i

[Physics]
  [NavierStokes]
    [TwoPhaseMixtureSegregated]
      [mixture]
        # Close the transfer on the solved area instead of prescribing it. The rate this creates is
        # 'mixture_interfacial_mass_transfer_rate', which the area equation is given.
        interfacial_area = 'interface_area'
        T_saturation = ${T_sat}
        interfacial_latent_heat = ${h_fg}
      []
    []
  []
[]

[FunctorMaterials]
  # The same closure, declared by hand from the same inputs the Physics hands it
  [gamma_check]
    type = WCNSFV2PInterfacialMassTransferFunctorMaterial
    interfacial_area = 'interface_area'
    fraction_dispersed = 'phase_2'
    T_fluid = 'T_fluid'
    T_saturation = ${T_sat}
    latent_heat = ${h_fg}
    rho_c = ${rho}
    mu_c = ${mu}
    k_c = ${k}
    u_slip = 'vel_slip_x'
    v_slip = 'vel_slip_y'
    interfacial_mass_transfer_name = 'gamma_check'
  []
  [gamma_mismatch]
    type = ParsedFunctorMaterial
    property_name = 'gamma_mismatch'
    expression = 'gamma_check - mixture_interfacial_mass_transfer_rate'
    functor_names = 'gamma_check mixture_interfacial_mass_transfer_rate'
  []
[]

# The rates are evaluated at cell centres, as the kernels evaluate them; the closure cannot be formed
# at quadrature points because the slip velocity it contains carries a time derivative
[AuxVariables]
  [gamma_cell]
    type = MooseLinearVariableFVReal
    [AuxKernel]
      type = FunctorAux
      functor = 'mixture_interfacial_mass_transfer_rate'
      execute_on = 'INITIAL TIMESTEP_END'
    []
  []
  [gamma_mismatch_cell]
    type = MooseLinearVariableFVReal
    [AuxKernel]
      type = FunctorAux
      functor = 'gamma_mismatch'
      execute_on = 'INITIAL TIMESTEP_END'
    []
  []
[]

[Postprocessors]
  [gamma_rate]
    type = ElementAverageValue
    variable = gamma_cell
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # The hand-declared closure against the one the Physics builds: identically zero
  [gamma_closure_mismatch]
    type = ElementAverageValue
    variable = gamma_mismatch_cell
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
