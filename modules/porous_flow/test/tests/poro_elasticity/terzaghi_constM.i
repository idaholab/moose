# Terzaghi's problem of consolodation of a drained medium
#
# A saturated soil sample sits in a bath of water.
# It is constrained on its sides, and bottom.
# Its sides and bottom are also impermeable.
# Initially it is unstressed.
# A normal stress, q, is applied to the soil's top.
# The soil then slowly compresses as water is squeezed
# out from the sample from its top (the top BC for
# the porepressure is porepressure = 0).
#
# See, for example.  Section 2.2 of the online manuscript
# Arnold Verruijt "Theory and Problems of Poroelasticity" Delft University of Technology 2013
# but note that the "sigma" in that paper is the negative
# of the stress in TensorMechanics
#
# Here are the problem's parameters, and their values:
# Soil height.  h = 10
# Soil's Lame lambda.  la = 2
# Soil's Lame mu, which is also the Soil's shear modulus.  mu = 3
# Soil bulk modulus.  K = la + 2*mu/3 = 4
# Soil confined compressibility.  m = 1/(K + 4mu/3) = 0.125
# Soil bulk compliance.  1/K = 0.25
# Fluid bulk modulus.  Kf = 8
# Fluid bulk compliance.  1/Kf = 0.125
# Fluid mobility (soil permeability/fluid viscosity).  k = 1.5
# Soil initial porosity.  phi0 = 0.1
# Biot coefficient.  alpha = 0.6
# Soil initial storativity, which is the reciprocal of the initial Biot modulus.  S = phi0/Kf + (alpha - phi0)(1 - alpha)/K = 0.0625
# Consolidation coefficient.  c = k/(S + alpha^2 m) = 13.95348837
# Normal stress on top.  q = 1
# Initial porepressure, resulting from instantaneous application of q, assuming corresponding instantaneous increase of porepressure (Note that this is calculated by MOOSE: we only need it for the analytical solution).  p0 = alpha*m*q/(S + alpha^2 m) = 0.69767442
# Initial vertical displacement (down is positive), resulting from instantaneous application of q (Note this is calculated by MOOSE: we only need it for the analytical solution).  uz0 = q*m*h*S/(S + alpha^2 m)
# Final vertical displacement (down in positive) (Note this is calculated by MOOSE: we only need it for the analytical solution).  uzinf = q*m*h
#
# The solution for porepressure is
# P = 4*p0/\pi \sum_{k=1}^{\infty} \frac{(-1)^{k-1}}{2k-1} \cos ((2k-1)\pi z/(2h)) \exp(-(2k-1)^2 \pi^2 ct/(4 h^2))
# This series converges very slowly for ct/h^2 small, so in that domain
# P = p0 erf( (1-(z/h))/(2 \sqrt(ct/h^2)) )
#
# The degree of consolidation is defined as
# U = (uz - uz0)/(uzinf - uz0)
# where uz0 and uzinf are defined above, and
# uz = the vertical displacement of the top (down is positive)
# U = 1 - (8/\pi^2)\sum_{k=1}^{\infty} \frac{1}{(2k-1)^2} \exp(-(2k-1)^2 \pi^2 ct/(4 h^2))

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 10
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = 0
  zmax = 10
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
  block = 0
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure disp_x disp_y disp_z'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.8
    alpha = 1
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [porepressure]
  []
[]

[BCs]
  [confinex]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 'left right'
  []
  [confiney]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = 'bottom top'
  []
  [basefixed]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = back
  []
  [topdrained]
    type = DirichletBC
    variable = porepressure
    value = 0
    boundary = front
  []
  [topload]
    type = NeumannBC
    variable = disp_z
    value = -1
    boundary = front
  []
[]

[Kernels]
  [grad_stress_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  []
  [grad_stress_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  []
  [grad_stress_z]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
  []
  [poro_x]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.6
    variable = disp_x
    component = 0
  []
  [poro_y]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.6
    variable = disp_y
    component = 1
  []
  [poro_z]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.6
    component = 2
    variable = disp_z
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
  [flux]
    type = PorousFlowAdvectiveFlux
    variable = porepressure
    gravity = '0 0 0'
    fluid_component = 0
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 8
    density0 = 1
    thermal_expansion = 0
    viscosity = 0.96
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '2 3'
    # bulk modulus is lambda + 2*mu/3 = 2 + 2*3/3 = 4
    fill_method = symmetric_isotropic
  []
  [strain]
    type = ComputeSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [eff_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
  []
  [vol_strain]
    type = PorousFlowVolumetricStrain
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
    type = PorousFlowPorosityHMBiotModulus
    porosity_zero = 0.1
    biot_coefficient = 0.6
    solid_bulk = 4
    constant_fluid_bulk_modulus = 8
    constant_biot_modulus = 16
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.5 0 0   0 1.5 0   0 0 1.5'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 0 # unimportant in this fully-saturated situation
    phase = 0
  []
[]

[Postprocessors]
  [p0]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = porepressure
    use_displaced_mesh = false
  []
  [p1]
    type = PointValue
    outputs = csv
    point = '0 0 1'
    variable = porepressure
    use_displaced_mesh = false
  []
  [p2]
    type = PointValue
    outputs = csv
    point = '0 0 2'
    variable = porepressure
    use_displaced_mesh = false
  []
  [p3]
    type = PointValue
    outputs = csv
    point = '0 0 3'
    variable = porepressure
    use_displaced_mesh = false
  []
  [p4]
    type = PointValue
    outputs = csv
    point = '0 0 4'
    variable = porepressure
    use_displaced_mesh = false
  []
  [p5]
    type = PointValue
    outputs = csv
    point = '0 0 5'
    variable = porepressure
    use_displaced_mesh = false
  []
  [p6]
    type = PointValue
    outputs = csv
    point = '0 0 6'
    variable = porepressure
    use_displaced_mesh = false
  []
  [p7]
    type = PointValue
    outputs = csv
    point = '0 0 7'
    variable = porepressure
    use_displaced_mesh = false
  []
  [p8]
    type = PointValue
    outputs = csv
    point = '0 0 8'
    variable = porepressure
    use_displaced_mesh = false
  []
  [p9]
    type = PointValue
    outputs = csv
    point = '0 0 9'
    variable = porepressure
    use_displaced_mesh = false
  []
  [p99]
    type = PointValue
    outputs = csv
    point = '0 0 10'
    variable = porepressure
    use_displaced_mesh = false
  []
  [zdisp]
    type = PointValue
    outputs = csv
    point = '0 0 10'
    variable = disp_z
    use_displaced_mesh = false
  []
  [dt]
    type = FunctionValuePostprocessor
    outputs = console
    function = if(0.5*t<0.1,0.5*t,0.1)
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = 0
  end_time = 10
  [TimeStepper]
    type = PostprocessorDT
    postprocessor = dt
    dt = 0.0001
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = terzaghi_constM
  [csv]
    type = CSV
  []
[]
