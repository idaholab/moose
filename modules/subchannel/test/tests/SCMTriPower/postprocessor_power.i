# Assigns q_prime with SCMTriPowerIC from a power supplied by a postprocessor, without solving for
# the coolant, and integrates q_prime over the pins to check that it carries that power
power = 7000.0 # W

# Seven pins: the center pin and one ring of six
[TriSubChannelMesh]
  [subchannel]
    type = SCMTriAssemblyMeshGenerator
    nrings = 2
    n_cells = 10
    heated_length = 1.0
    flat_to_flat = 0.0769620
    pin_diameter = 0.024638
    pitch = 0.027813
    dwire = 0.0
    hwire = 0.0
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    expression = 'sin(pi*z)'
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[SubChannel]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = 1.01325e5 # Pa
  compute_density = false
  compute_viscosity = false
  compute_power = false
  # Only q_prime is assigned and checked, so the coolant is not solved for
  solve = false
  friction_closure = uctd_friction
  mixing_closure = kim_chung_mixing
  pin_HTC_closure = kazimi_carelli_htc
[]

[SCMClosures]
  [uctd_friction]
    type = SCMFrictionUpgradedChengTodreas
  []
  [kim_chung_mixing]
    type = SCMMixingKimAndChung
  []
  [kazimi_carelli_htc]
    type = SCMHTCKazimiCarelli
  []
[]

[ICs]
  [q_prime_IC]
    type = SCMTriPowerIC
    variable = q_prime
    power = power_pp
    filename = 'seven_pin_power_profile.txt'
    axial_heat_rate = axial_heat_rate
  []
[]

[Postprocessors]
  [power_pp]
    type = FunctionValuePostprocessor
    function = ${power}
    execute_on = 'initial'
  []
  # Trapezoidal integral of q_prime over the pins, as used by the subchannel solver
  [q_prime_power]
    type = SCMPinPowerPostprocessor
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
