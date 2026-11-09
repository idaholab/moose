# Assigns q_prime with SCMQuadPowerIC from a power supplied by a postprocessor, without solving for
# the coolant, and integrates q_prime over the pins to check that it carries that power
power = 30000.0 # W

# Two pins at x = -pitch/2 and x = +pitch/2
[QuadSubChannelMesh]
  [subchannel]
    type = SCMQuadAssemblyMeshGenerator
    nx = 3
    ny = 2
    n_cells = 10
    pitch = 0.0126
    pin_diameter = 0.00950
    side_gap = 0.00095
    heated_length = 1.0
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    expression = 'sin(pi*z)'
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
  P_out = 4.923e6 # Pa
  compute_density = false
  compute_viscosity = false
  compute_power = false
  # Only q_prime is assigned and checked, so the coolant is not solved for
  solve = false
  friction_closure = 'MATRA'
  mixing_closure = 'constant_beta'
  pin_HTC_closure = 'Dittus-Boelter'
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
  [constant_beta]
    type = SCMMixingConstantBeta
    beta = 0.006
    CT = 2.6
  []
  [Dittus-Boelter]
    type = SCMHTCDittusBoelter
  []
[]

[ICs]
  [q_prime_IC]
    type = SCMQuadPowerIC
    variable = q_prime
    power = power_pp
    filename = 'two_pin_power_profile.txt'
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
