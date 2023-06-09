# Test dispersive part of FVPorousFlowDispersiveFlux kernel by setting diffusion
# coefficients to zero. A pressure gradient is applied over the mesh to give a
# uniform velocity. Gravity is set to zero.
# Mass fraction is set to 1 on the left hand side and 0 on the right hand side.

[Mesh]
  type = GeneratedMesh
  dim = 2
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
    type = FunctionIC
    variable = pp
    function = pic
  []
  [massfrac0]
    type = ConstantIC
    variable = massfrac0
    value = 0
  []
[]

[Functions]
  [pic]
    type = ParsedFunction
    expression = '1.1e5-x*1e3'
  []
[]

[FVBCs]
  [xleft]
    type = FVDirichletBC
    value = 1
    variable = massfrac0
    boundary = left
  []
  [xright]
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
    value = 1.1e5
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [adv0]
    type = FVPorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
  []
  [diff0]
    type = FVPorousFlowDispersiveFlux
    variable = pp
    disp_trans = 0
    disp_long = 0.2
  []
  [mass1]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 1
    variable = massfrac0
  []
  [adv1]
    type = FVPorousFlowAdvectiveFlux
    fluid_component = 1
    variable = massfrac0
  []
  [diff1]
    type = FVPorousFlowDispersiveFlux
    fluid_component = 1
    variable = massfrac0
    disp_trans = 0
    disp_long = 0.2
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
    bulk_modulus = 1e9
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
    diffusion_coeff = '0 0'
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
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type'
    petsc_options_value = 'gmres      asm      lu           NONZERO'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 3e2
  dtmax = 100
  nl_abs_tol = 1e-12
  [TimeStepper]
    type = IterationAdaptiveDT
    growth_factor = 2
    cutback_factor = 0.5
    dt = 10
  []
[]

[VectorPostprocessors]
  [xmass]
    type = ElementValueSampler
    sort_by = id
    variable = 'massfrac0 velocity'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = final
  []
[]
