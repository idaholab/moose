# Same as pp_generation.i, but using an Action
#
# A sample is constrained on all sides and its boundaries are
# also impermeable.  Fluid is pumped into the sample via a
# volumetric source (ie kg/second per cubic meter), and the
# rise in porepressure is observed.
#
# Source = s  (units = kg/m^3/second)
#
# Expect:
# fluid_mass = mass0 + s*t
# stress = 0 (remember this is effective stress)
# Porepressure = fluid_bulk*log(fluid_mass_density/density_P0), where fluid_mass_density = fluid_mass*porosity
# porosity = biot+(phi0-biot)*exp(pp(biot-1)/solid_bulk)
#
# Parameters:
# Biot coefficient = 0.3
# Phi0 = 0.1
# Solid Bulk modulus = 2
# fluid_bulk = 13
# density_P0 = 1

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
  block = 0
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

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0.0
    bulk_modulus = 13.0
    viscosity = 1.0
    density0 = 1.0
  []
[]

[PorousFlowUnsaturated]
  coupling_type = HydroMechanical
  displacements = 'disp_x disp_y disp_z'
  porepressure = porepressure
  biot_coefficient = 0.3
  gravity = '0 0 0'
  fp = the_simple_fluid
  van_genuchten_alpha = 1.0
  van_genuchten_m = 0.8
  relative_permeability_type = Corey
  relative_permeability_exponent = 0.0
  save_component_rate_in = nodal_kg_per_s
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
  [confinez]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'back front'
  []
[]

[Kernels]
  [source]
    type = BodyForce
    function = 0.1
    variable = porepressure
  []
[]

[AuxVariables]
  [porosity]
    order = CONSTANT
    family = MONOMIAL
  []
  [nodal_kg_per_s]
  []
[]

[AuxKernels]
  [porosity]
    type = PorousFlowPropertyAux
    variable = porosity
    property = porosity
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1 1.5'
    # bulk modulus is lambda + 2*mu/3 = 1 + 2*1.5/3 = 2
    fill_method = symmetric_isotropic
  []
  [strain]
    type = ComputeSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [porosity]
    type = PorousFlowPorosity
    fluid = true
    mechanical = true
    porosity_zero = 0.1
    biot_coefficient = 0.3
    solid_bulk = 2
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0   0 1 0   0 0 1' # unimportant
  []
[]

[Functions]
  [porosity_analytic]
    type = ParsedFunction
    expression = 'biot+(phi0-biot)*exp(pp*(biot-1)/bulk)'
    symbol_names = 'biot phi0 pp bulk'
    symbol_values = '0.3 0.1 p0 2'
  []
[]

[Postprocessors]
  [nodal_kg_per_s]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = nodal_kg_per_s
  []
  [fluid_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
  [porosity]
    type = PointValue
    outputs = 'console csv'
    point = '0 0 0'
    variable = porosity
  []
  [p0]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = porepressure
  []
  [porosity_analytic]
    type = FunctionValuePostprocessor
    function = porosity_analytic
  []
  [zdisp]
    type = PointValue
    outputs = csv
    point = '0 0 0.5'
    variable = disp_z
  []
  [stress_xx]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_xx
  []
  [stress_yy]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_yy
  []
  [stress_zz]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_zz
  []

[]


[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-14 1E-10 10000'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = 0
  end_time = 10
  dt = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = pp_generation_action
  csv = true
[]
