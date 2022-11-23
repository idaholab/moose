# checking that the PorousFlowDarcyVelocityComponent AuxKernel works as expected
# for the fully-saturated case (relative-permeability = 1)
# There is one element, of unit size.  The pressures and fluid densities at the qps are:
# (x,y,z)=( 0.211325 , 0.211325 , 0.211325 ).  p = 1.479   rho = 3.217
# (x,y,z)=( 0.788675 , 0.211325 , 0.211325 ).  p = 2.057   rho = 4.728
# (x,y,z)=( 0.211325 , 0.788675 , 0.211325 ).  p = 2.634   rho = 6.947
# (x,y,z)=( 0.788675 , 0.788675 , 0.211325 ).  p = 3.211   rho = 10.208
# (x,y,z)=( 0.211325 , 0.211325 , 0.788675 ).  p = 3.789   rho = 15.001
# (x,y,z)=( 0.788675 , 0.211325 , 0.788675 ).  p = 4.367   rho = 22.043
# (x,y,z)=( 0.211325 , 0.788675 , 0.788675 ).  p = 4.943   rho = 32.392
# (x,y,z)=( 0.788675 , 0.788675 , 0.788675 ).  p = 5.521   rho = 47.599
# Average density = 17.7668
# grad(P) = (1, 2, 4)
# with permeability = diag(1, 2, 3) and gravity = (1, -2, 3) and viscosity = 3.2
# So Darcy velocity = (5.23963, -23.4585, 46.2192)

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '1 -2 3'
[]

[Variables]
  [pp]
  []
[]

[ICs]
  [pinit]
    type = FunctionIC
    function = x+2*y+4*z
    variable = pp
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
[]

[AuxVariables]
  [vel_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [vel_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [vel_z]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [vel_x]
    type = PorousFlowDarcyVelocityComponent
    variable = vel_x
    component = x
    fluid_phase = 0
  []
  [vel_y]
    type = PorousFlowDarcyVelocityComponent
    variable = vel_y
    component = y
    fluid_phase = 0
  []
  [vel_z]
    type = PorousFlowDarcyVelocityComponent
    variable = vel_z
    component = z
    fluid_phase = 0
 []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1.5
    viscosity = 3.2
    density0 = 1.2
    thermal_expansion = 0
  []
[]

[Postprocessors]
  [vel_x]
    type = PointValue
    variable = vel_x
    point = '0.5 0.5 0.5'
  []
  [vel_y]
    type = PointValue
    variable = vel_y
    point = '0.5 0.5 0.5'
  []
  [vel_z]
    type = PointValue
    variable = vel_z
    point = '0.5 0.5 0.5'
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
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
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  nl_abs_tol = 1e-16
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = darcy_velocity
  csv = true
[]
