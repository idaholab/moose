# Illustrates desorption works as planned.
#
# A mesh contains 3 elements in arranged in a line.
# The central element contains desorped fluid.
# This desorps to the nodes of that element.
#
# In the central element, of volume V, the following occurs.
# The initial porepressure=1, and concentration=1.
# The initial mass of fluid is
# V * (2 * porosity * density + (1 - porosity) * concentration)
# = V * 1.289547
# Notice the factor of "2" in the porespace contribution:
# it is because the porepressure is evaluated at nodes, so
# the nodes on the exterior of the centre_block have
# nodal-volume contributions from the elements not in centre_block.
#
# The mass-conservation equation reads
# 2 * porosity * density + (1 - porosity) * concentration = 1.289547
# and the desorption equation reads
# d( (1-porosity)C )/dt = - (1/tau)(C - dens_L * P / (P_L + P))
# where C = concentration, P = porepressure, P_L = Langmuir pressure
# dens_L = Langmuir density, tau = time constant.
# Using the mass-conservation equation in the desorption equation
# yields a nonlinear equation of P.  For dt=1, and the numerical values
# given below this yields
# P = 1.83697
# and
# C = 0.676616
# The desired result is achieved by MOOSE
[Mesh]
  type = FileMesh
  file = three_eles.e
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./pp]
  [../]
  [./conc]
    family = MONOMIAL
    order = CONSTANT
    block = centre_block
  [../]
[]

[ICs]
  [./p_ic]
    type = ConstantIC
    variable = pp
    value = 1.0
  [../]
  [./conc_ic]
    type = ConstantIC
    variable = conc
    value = 1.0
    block = centre_block
  [../]
[]

[Kernels]
  [./porespace_mass_dot]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
  [./fluid_flow]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
     variable = pp
     gravity = '0 0 0'
  [../]
  [./desorped_mass_dot]
    type = PorousFlowDesorpedMassTimeDerivative
    block = centre_block
    conc_var = conc
    variable = pp
  [../]
  [./desorped_mass_dot_conc_var]
    type = PorousFlowDesorpedMassTimeDerivative
    block = centre_block
    conc_var = conc
    variable = conc
  [../]
  [./flow_from_matrix]
    type = DesorptionFromMatrix
    block = centre_block
    variable = conc
    pressure_var = pp
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp conc'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./lang_stuff]
    type = LangmuirMaterial
    block = centre_block
    one_over_adsorption_time_const = 10.0
    one_over_desorption_time_const = 10.0
    langmuir_density = 1
    langmuir_pressure = 1
    pressure_var = pp
    conc_var = conc
  [../]

  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    at_nodes = true
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens_all_qp]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
    at_nodes = true
  [../]
  [./porosity_qp]
    type = PorousFlowPorosityConst
    porosity = 0.1
    block = centre_block
  [../]

  [./temperature_qp]
    type = PorousFlowTemperature
  [../]
  [./ppss_qp]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1
    m = 0.5
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0 0 0  0 0 0  0 0 0'
  [../]
  [./visc]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityFLAC
    at_nodes = true
    m = 1
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  exodus = true
[]
