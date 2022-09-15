# The sample is an annulus in RZ coordinates.
# Roller BCs are applied to the rmin, top and bottom boundaries
# A constant displacement is applied to the outer boundary: disp_r = -0.01 * t * (r - rmin)/(rmax - rmin).
# There is no fluid flow.
# Fluid mass conservation is checked.
#
# The flag volumetric_locking_correction = true is set for the strain calculator,
# which ensures that the volumetric strain is uniform throughout the element
#
# Theoretically,
# volumetric_strain = volume / volume0 - 1 = ((rmax - 0.01*t)^2 - rmin^2) / (rmax^2 - rmin^2) - 1
# However, with ComputeAxisymmetricRZSmallStrain, strain_rr = -0.01 * t / (rmax - rmin)
# and strain_tt = disp_r / r = -0.01 * t * (1 - rmin / r_qp) / (rmax - rmin), where r_qp is the radius of the quadpoint
# With volumetric_locking_correction = true, r_qp = (rmax - rmin) / 2.
# The volumetric strain is
# epv = -0.01 * t * (2 - rmin / r_qp) / (rmax - rmin)
# and volume = volume0 * (1 + epv)
#
# Fluid conservation reads
# volume0 * rho0 * exp(P0/bulk) = volume * rho0 * exp(P/bulk), so
# P - P0 = bulk * log(volume0 / volume) = 0.5 * log(1 / (1 + epv))

# With rmax = 2 and rmin = 1
# fluid_mass = volume0 * rho0 * exp(P0/bulk) = pi*3 * 1 * exp(0.1/0.5) = 11.51145


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 1
  xmax = 2
  ymin = -0.5
  ymax = 0.5
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
  PorousFlowDictator = dictator
  block = 0
  biot_coefficient = 0.3
[]

[Variables]
  [disp_r]
  []
  [disp_z]
  []
  [porepressure]
    initial_condition = 0.1
  []
[]

[BCs]
  [plane_strain]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'bottom top'
  []
  [rmin_fixed]
    type = DirichletBC
    variable = disp_r
    value = 0
    boundary = left
  []
  [contract]
    type = FunctionDirichletBC
    variable = disp_r
    function = -0.01*t
    boundary = right
  []
[]

[Kernels]
  [grad_stress_r]
    type = StressDivergenceRZTensors
    variable = disp_r
    component = 0
  []
  [grad_stress_z]
    type = StressDivergenceRZTensors
    variable = disp_z
    component = 1
  []
  [poro_r]
    type = PorousFlowEffectiveStressCoupling
    variable = disp_r
    component = 0
  []
  [poro_z]
    type = PorousFlowEffectiveStressCoupling
    variable = disp_z
    component = 1
  []
  [poro_vol_exp]
    type = PorousFlowMassVolumetricExpansion
    variable = porepressure
    fluid_component = 0
  []
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = porepressure
  []
[]

[AuxVariables]
  [stress_rr]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_rz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_tt]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_rr]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_tt]
    order = CONSTANT
    family = MONOMIAL
  []
  [vol_strain]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_rr]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_rr
    index_i = 0
    index_j = 0
  []
  [stress_rz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_rz
    index_i = 0
    index_j = 1
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 1
    index_j = 1
  []
  [stress_tt]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_tt
    index_i = 2
    index_j = 2
  []
  [strain_rr]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_rr
    index_i = 0
    index_j = 0
  []
  [strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_i = 1
    index_j = 1
  []
  [strain_tt]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_tt
    index_i = 2
    index_j = 2
  []
  [vol_strain]
    type = MaterialRealAux
    property = PorousFlow_total_volumetric_strain_qp
    variable = vol_strain
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 0.5
    density0 = 1
    thermal_expansion = 0
    viscosity = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1 1.5'
    # bulk modulus is lambda + 2*mu/3 = 1 + 2*1.5/3 = 2
    fill_method = symmetric_isotropic
  []
  [strain]
    type = ComputeAxisymmetricRZSmallStrain
    volumetric_locking_correction = true # the strain will be the same at every qp of the element
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [vol_strain]
    type = PorousFlowVolumetricStrain
  []
  [eff_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = porepressure
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.5 0 0   0 0.5 0   0 0 0.5'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure disp_r disp_z'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[Postprocessors]
  [p0]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'initial timestep_end'
    point = '1.0 0 0'
    variable = porepressure
  []
  [vol_strain]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'initial timestep_end'
    point = '1 0 0'
    variable = vol_strain
  []
  [strain_rr]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'initial timestep_end'
    point = '1 0 0'
    variable = strain_rr
  []
  [strain_zz]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'initial timestep_end'
    point = '1 0 0'
    variable = strain_zz
  []
  [strain_tt]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'initial timestep_end'
    point = '1 0 0'
    variable = strain_tt
  []
  [rdisp]
    type = PointValue
    outputs = csv
    point = '2 0 0'
    use_displaced_mesh = false
    variable = disp_r
  []
  [stress_rr]
    type = PointValue
    outputs = csv
    point = '1 0 0'
    variable = stress_rr
  []
  [stress_zz]
    type = PointValue
    outputs = csv
    point = '1 0 0'
    variable = stress_zz
  []
  [stress_tt]
    type = PointValue
    outputs = csv
    point = '1 0 0'
    variable = stress_tt
  []
  [fluid_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
    outputs = 'console csv'
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-14 1E-8 10000'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = 0
  end_time = 10
  dt = 2
[]

[Outputs]
  execute_on = 'initial timestep_end'
  [csv]
    type = CSV
  []
[]
