# Verification of a prescribed interfacial mass transfer against the exact solution.
#
# The closed quiescent box of interfacial-mass-transfer-base.i, with the transfer rate prescribed as
# a constant Gamma_0 instead of closed on the interfacial area. Every field stays uniform, so the
# phase and energy equations reduce to
#
#   rho_d d(alpha)/dt = Gamma_0 ,      rho_m cp_m dT/dt = -Gamma_0 h_fg ,
#
# whose solutions are linear in time, and the implicit Euler step reproduces a linear solution
# exactly. The assertions are therefore
#
#   alpha(t) = alpha_0 + Gamma_0 t / rho_d ,      T(t) = T_0 - Gamma_0 h_fg t / (rho_m cp_m) ,
#
# to round-off, with rho_m = rho_d and cp_m = cp_d here because the phases are identical in both.
# The interfacial area equation is driven by the same prescribed rate, so its invariant
# chi / alpha^(2/3) holds as in the closed case, to the order of the time integrator.
#
# What this checks is the coupling of a supplied rate into the phase, energy and area equations:
# the sign and scaling of the phase source, the latent heat sink on the energy equation, and the
# phase change source of the area equation. The closure itself is verified separately.

# The prescribed rate, in kg per cubic metre per second. Takes the phase fraction from 0.05 to
# 0.25 over the run, a factor of five, as the closed case does.
gamma_0 = 100.0
gamma_name = ${gamma_0}

!include interfacial-mass-transfer-base.i

[Physics]
  [NavierStokes]
    [TwoPhaseMixtureSegregated]
      [mixture]
        interfacial_mass_transfer = ${gamma_0}
        interfacial_latent_heat = ${h_fg}
      []
    []
  []
[]

[Postprocessors]
  # Departures from the exact linear solutions, both identically zero
  [alpha_error]
    type = ParsedPostprocessor
    expression = 'alpha - (${alpha_initial} + ${gamma_0} * t / ${rho_d})'
    pp_names = 'alpha'
    use_t = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [temperature_error]
    type = ParsedPostprocessor
    expression = 'temperature - (${T_initial} - ${gamma_0} * ${h_fg} * t / (${rho} * ${cp}))'
    pp_names = 'temperature'
    use_t = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
