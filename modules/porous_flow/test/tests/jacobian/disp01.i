# Test the Jacobian of the dispersive contribution to the diffusive component of
# the PorousFlowDisperiveFlux kernel. By setting disp_long and disp_trans to the same
# non-zero value, and diffusion to zero (by setting tortuosity to zero), the purely
# dispersive component of the flux is zero, and the only flux is due to the contribution
# from disp_trans on the diffusive flux.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  xmin = 0
  xmax = 1
  ny = 1
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
  [massfrac0]
  []
[]

[ICs]
  [pp]
    type = RandomIC
    variable = pp
    max = 2e1
    min = 1e1
  []
  [massfrac0]
    type = RandomIC
    variable = massfrac0
    min = 0
    max = 1
  []
[]

[Kernels]
  [diff0]
    type = PorousFlowDispersiveFlux
    fluid_component = 0
    variable = pp
    gravity = '1 0 0'
    disp_long = 0.1
    disp_trans = 0.1
  []
  [diff1]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = massfrac0
    gravity = '1 0 0'
    disp_long = 0.1
    disp_trans = 0.1
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
    density0 = 10
    thermal_expansion = 0
    viscosity = 1
  []
[]

[Materials]
  [temp]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac0'
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [poro]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [diff]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1e-2 1e-1'
    tortuosity = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
[]

[Preconditioning]
  active = smp
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  exodus = false
[]
