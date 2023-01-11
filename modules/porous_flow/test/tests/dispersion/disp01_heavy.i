# Test dispersive part of PorousFlowDispersiveFlux kernel by setting diffusion
# coefficients to zero. A pressure gradient is applied over the mesh to give a
# uniform velocity. Gravity is set to zero.
# Mass fraction is set to 1 on the left hand side and 0 on the right hand side.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 200
  xmax = 10
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
  compute_enthalpy = false
  compute_internal_energy = false
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
    expression = 1.1e5-x*1e3
  []
[]

[BCs]
  [xleft]
    type = DirichletBC
    value = 1
    variable = massfrac0
    boundary = left
  []
  [xright]
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
    value = 1.1e5
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
    variable = pp
    disp_trans = 0
    disp_long = 0.2
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
    diffusion_coeff = '0 0'
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
  end_time = 1e3
  dtmax = 10
  [TimeStepper]
    type = IterationAdaptiveDT
    growth_factor = 1.5
    cutback_factor = 0.5
    dt = 1
  []
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
