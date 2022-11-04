# Identical to pp_generation_unconfined_fullysat_volume.i but using an Action
#
# A sample is constrained on all sides, except its top
# and its boundaries are
# also impermeable.  Fluid is pumped into the sample via a
# volumetric source (ie m^3/second per cubic meter), and the
# rise in the top surface, porepressure, and stress are observed.
#
# In the standard poromechanics scenario, the Biot Modulus is held
# fixed and the source has units 1/s.  Then the expected result
# is
# strain_zz = disp_z = BiotCoefficient*BiotModulus*s*t/((bulk + 4*shear/3) + BiotCoefficient^2*BiotModulus)
# porepressure = BiotModulus*(s*t - BiotCoefficient*strain_zz)
# stress_xx = (bulk - 2*shear/3)*strain_zz   (remember this is effective stress)
# stress_zz = (bulk + 4*shear/3)*strain_zz   (remember this is effective stress)
#
# In standard porous_flow, everything is based on mass, eg the source has
# units kg/s/m^3.  This is discussed in the other pp_generation_unconfined
# models.  In this test, we use the FullySaturated Kernel and set
# multiply_by_density = false
# meaning the fluid Kernel has units of volume, and the source, s, has units 1/time
#
# The ratios are:
# stress_xx/strain_zz = (bulk - 2*shear/3) = 1 (for the parameters used here)
# stress_zz/strain_zz = (bulk + 4*shear/3) = 4 (for the parameters used here)
# porepressure/strain_zz = 13.3333333 (for the parameters used here)
#
# Expect
# disp_z = 0.3*10*s*t/((2 + 4*1.5/3) + 0.3^2*10) = 0.612245*s*t
# porepressure = 10*(s*t - 0.3*0.612245*s*t) = 8.163265*s*t
# stress_xx = (2 - 2*1.5/3)*0.612245*s*t = 0.612245*s*t
# stress_zz = (2 + 4*shear/3)*0.612245*s*t = 2.44898*s*t
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
  [confinez]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'back'
  []
[]


[Kernels]
  [source]
    type = BodyForce
    function = 0.1
    variable = porepressure
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0.0
    bulk_modulus = 3.3333333333
    viscosity = 1.0
    density0 = 1.0
  []
[]

[PorousFlowBasicTHM]
  coupling_type = HydroMechanical
  displacements = 'disp_x disp_y disp_z'
  multiply_by_density = false
  porepressure = porepressure
  biot_coefficient = 0.3
  gravity = '0 0 0'
  fp = the_simple_fluid
  save_component_rate_in = nodal_m3_per_s
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
    displacements = 'disp_x disp_y disp_z'
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [porosity]
    type = PorousFlowPorosityConst # the "const" is irrelevant here: all that uses Porosity is the BiotModulus, which just uses the initial value of porosity
    porosity = 0.1
    PorousFlowDictator = dictator
  []
  [biot_modulus]
    type = PorousFlowConstantBiotModulus
    PorousFlowDictator = dictator
    biot_coefficient = 0.3
    fluid_bulk_modulus = 3.3333333333
    solid_bulk_compliance = 0.5
  []
  [permeability_irrelevant]
    type = PorousFlowPermeabilityConst
    PorousFlowDictator = dictator
    permeability = '1.5 0 0   0 1.5 0   0 0 1.5'
  []
[]

[AuxVariables]
  [nodal_m3_per_s]
  []
[]

[Postprocessors]
  [nodal_m3_per_s]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = nodal_m3_per_s
  []
  [p0]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = porepressure
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
  [stress_xx_over_strain]
    type = FunctionValuePostprocessor
    function = stress_xx_over_strain_fcn
    outputs = csv
  []
  [stress_zz_over_strain]
    type = FunctionValuePostprocessor
    function = stress_zz_over_strain_fcn
    outputs = csv
  []
  [p_over_strain]
    type = FunctionValuePostprocessor
    function = p_over_strain_fcn
    outputs = csv
  []
[]

[Functions]
  [stress_xx_over_strain_fcn]
    type = ParsedFunction
    expression = a/b
    symbol_names = 'a b'
    symbol_values = 'stress_xx zdisp'
  []
  [stress_zz_over_strain_fcn]
    type = ParsedFunction
    expression = a/b
    symbol_names = 'a b'
    symbol_values = 'stress_zz zdisp'
  []
  [p_over_strain_fcn]
    type = ParsedFunction
    expression = a/b
    symbol_names = 'a b'
    symbol_values = 'p0 zdisp'
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
  file_base = pp_generation_unconfined_basicthm
  [csv]
    type = CSV
  []
[]
