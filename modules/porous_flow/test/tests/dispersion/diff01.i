# Test diffusive part of PorousFlowDispersiveFlux kernel by setting dispersion
# coefficients to zero. Pressure is held constant over the mesh, and gravity is
# set to zero so that no advective transport of mass takes place.
# Mass fraction is set to 1 on the left hand side and 0 on the right hand side.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 10
  bias_x = 1.1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pp]
  []
  [massfrac0]
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
    type = PorousFlowDarcyVelocityComponent
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

[BCs]
  [left]
    type = DirichletBC
    value = 1
    variable = massfrac0
    boundary = left
  []
  [right]
    type = DirichletBC
    value = 0
    variable = massfrac0
    boundary = right
  []
  [pright]
    type = DirichletBC
    variable = pp
    boundary = right
    value = 1e5
  []
  [pleft]
    type = DirichletBC
    variable = pp
    boundary = left
    value = 1e5
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [adv0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
  []
  [diff0]
    type = PorousFlowDispersiveFlux
    fluid_component = 0
    variable = pp
    disp_trans = 0
    disp_long = 0
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = massfrac0
  []
  [adv1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = massfrac0
  []
  [diff1]
    type = PorousFlowDispersiveFlux
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
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = massfrac0
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [poro]
    type = PorousFlowPorosityConst
    porosity = 0.3
  []
  [diff]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1 1'
    tortuosity = 0.1
  []
  [relp]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
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
    type = NodalValueSampler
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
