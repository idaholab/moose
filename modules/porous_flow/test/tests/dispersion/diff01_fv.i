# Test diffusive part of FVPorousFlowDispersiveFlux kernel by setting dispersion
# coefficients to zero. Pressure is held constant over the mesh, and gravity is
# set to zero so that no advective transport of mass takes place.
# Mass fraction is set to 1 on the left hand side and 0 on the right hand side.

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
    xmax = 10
    bias_x = 1.2
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pp]
    type = MooseVariableFVReal
  []
  [massfrac0]
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [velocity]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxKernels]
  [velocity]
    type = ADPorousFlowDarcyVelocityComponent
    variable = velocity
    component = x
  []
[]

[ICs]
  [pp]
    type = ConstantIC
    variable = pp
    value = 1e5
  []
  [massfrac0]
    type = ConstantIC
    variable = massfrac0
    value = 0
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    value = 1
    variable = massfrac0
    boundary = left
  []
  [right]
    type = FVDirichletBC
    value = 0
    variable = massfrac0
    boundary = right
  []
  [pright]
    type = FVDirichletBC
    variable = pp
    boundary = right
    value = 1e5
  []
  [pleft]
    type = FVDirichletBC
    variable = pp
    boundary = left
    value = 1e5
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [diff0_pp]
    type = FVPorousFlowDispersiveFlux
    fluid_component = 0
    variable = pp
    disp_trans = 0
    disp_long = 0
  []
  [mass1]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 1
    variable = massfrac0
  []
  [diff1_x]
    type = FVPorousFlowDispersiveFlux
    fluid_component = 1
    variable = massfrac0
    disp_trans = 0
    disp_long = 0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp massfrac0'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1e7
    density0 = 1000
    viscosity = 0.001
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
  []
  [ppss]
    type = ADPorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = ADPorousFlowMassFraction
    mass_fraction_vars = massfrac0
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [poro]
    type = ADPorousFlowPorosityConst
    porosity = 0.3
  []
  [diff]
    type = ADPorousFlowDiffusivityConst
    diffusion_coeff = '1 1'
    tortuosity = 0.1
  []
  [relp]
    type = ADPorousFlowRelativePermeabilityConst
    phase = 0
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1e-9 0 0 0 1e-9 0 0 0 1e-9'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2             '
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  end_time = 20
[]

[VectorPostprocessors]
  [xmass]
    type = ElementValueSampler
    sort_by = id
    variable = massfrac0
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = final
  []
[]
