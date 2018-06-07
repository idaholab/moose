# Test the Jacobian of the diffusive component of the PorousFlowDisperiveFlux kernel for two phases.
# By setting disp_long and disp_trans to zero, the purely diffusive component of the flux
# can be isolated. Uses constant tortuosity and diffusion coefficients

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
  [./sgas]
  [../]
  [./massfrac0]
  [../]
[]

[AuxVariables]
  [./massfrac1]
  [../]
[]

[ICs]
  [./sgas]
    type = RandomIC
    variable = sgas
    max = 1
    min = 0
  [../]
  [./massfrac0]
    type = RandomIC
    variable = massfrac0
    min = 0
    max = 1
  [../]
  [./massfrac1]
    type = RandomIC
    variable = massfrac1
    min = 0
    max = 1
  [../]
[]

[Kernels]
  [./diff0]
    type = PorousFlowDispersiveFlux
    fluid_component = 0
    variable = sgas
    gravity = '1 0 0'
    disp_long = '0 0'
    disp_trans = '0 0'
  [../]
  [./diff1]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = massfrac0
    gravity = '1 0 0'
    disp_long = '0 0'
    disp_trans = '0 0'
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'sgas massfrac0'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
  [./pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  [../]
[]

[Modules]
  [./FluidProperties]
    [./simple_fluid0]
      type = SimpleFluidProperties
      bulk_modulus = 1e7
      density0 = 10
      thermal_expansion = 0
      viscosity = 1
    [../]
    [./simple_fluid1]
      type = SimpleFluidProperties
      bulk_modulus = 1e7
      density0 = 1
      thermal_expansion = 0
      viscosity = 0.1
    [../]
  [../]
[]

[Materials]
  [./temp]
    type = PorousFlowTemperature
    at_nodes = false
  [../]
  [./ppss]
    type = PorousFlow2PhasePS
    at_nodes = false
    phase0_porepressure = 1
    phase1_saturation = sgas
    capillary_pressure = pc
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = false
    mass_fraction_vars = 'massfrac0 massfrac1'
  [../]
  [./simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    at_nodes = false
    phase = 0
  [../]
  [./simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    at_nodes = false
    phase = 1
  [../]
  [./poro]
    type = PorousFlowPorosityConst
    porosity = 0.1
    at_nodes = false
  [../]
  [./diff]
    type = PorousFlowDiffusivityConst
     diffusion_coeff = '1e-2 1e-1 1e-2 1e-1'
     tortuosity = '0.1 0.2'
    at_nodes = false
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  [../]
  [./relperm0]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
    at_nodes = false
  [../]
  [./relperm1]
    type = PorousFlowRelativePermeabilityConst
    phase = 1
    at_nodes = false
  [../]
[]

[Preconditioning]
  active = smp
  [./smp]
    type = SMP
    full = true
  [../]
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
