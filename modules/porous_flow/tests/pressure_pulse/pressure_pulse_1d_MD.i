# Pressure pulse in 1D with 1 phase - transient
# Using the "MD" formulation (where primary variable is log(mass-density)
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator_UO = dictator
[]

[Variables]
  [./md]
    # initial porepressure = 2E6
    # so initial md = log(density0) + porepressure/bulk_modulus =
    initial_condition = 6.90875527898214
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    component_index = 0
    variable = md
  [../]
  [./flux]
    type = PorousFlowAdvectiveFlux
    variable = md
    gravity = '0 0 0'
    component_index = 0
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'md'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./ppss]
    type = PorousFlowMaterial1PhaseMD_Gaussian
    mass_density = md
    al = 1E-6 # this is irrelevant in this example
    density0 = 1000
    bulk_modulus = 2E9
  [../]
  [./massfrac]
    type = PorousFlowMaterialMassFractionBuilder
  [../]
  [./dens0]
    type = PorousFlowMaterialDensityConstBulk
    density0 = 1000
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowMaterialJoinerOld
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./dens_all_at_quadpoints]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    use_qps = true
  [../]
  [./porosity]
    type = PorousFlowMaterialPorosityConst
    porosity = 0.1
  [../]
  [./permeability]
    type = PorousFlowMaterialPermeabilityConst
    permeability = '1E-15 0 0 0 1E-15 0 0 0 1E-15'
  [../]
  [./relperm]
    type = PorousFlowMaterialRelativePermeabilityCorey
    n_j = 0
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_relative_permeability
  [../]
  [./visc0]
    type = PorousFlowMaterialViscosityConst
    viscosity = 1E-3
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_viscosity
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    boundary = left
    # BC porepressure = 3E6
    # so boundary md = log(density0) + porepressure/bulk_modulus =
    value = 6.90925527898214
    variable = md
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-12 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E3
  end_time = 1E4
[]

[AuxVariables]
  [./pp]
  [../]
[]

[AuxKernels]
  [./pp]
    type = ParsedAux
    function = '(md-6.9077552789821)*2.0E9'
    args = 'md'
    variable = pp
  [../]
[]


[Postprocessors]
  [./p000]
    type = PointValue
    variable = pp
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p010]
    type = PointValue
    variable = pp
    point = '10 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p020]
    type = PointValue
    variable = pp
    point = '20 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p030]
    type = PointValue
    variable = pp
    point = '30 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p040]
    type = PointValue
    variable = pp
    point = '40 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p050]
    type = PointValue
    variable = pp
    point = '50 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p060]
    type = PointValue
    variable = pp
    point = '60 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p070]
    type = PointValue
    variable = pp
    point = '70 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p080]
    type = PointValue
    variable = pp
    point = '80 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p090]
    type = PointValue
    variable = pp
    point = '90 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./p100]
    type = PointValue
    variable = pp
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  file_base = pressure_pulse_1d_MD
  print_linear_residuals = false
  [./csv]
    type = CSV
  [../]
[]
