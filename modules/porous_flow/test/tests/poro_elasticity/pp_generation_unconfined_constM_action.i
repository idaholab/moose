# This file uses a PorousFlowFullySaturated Action.  The equivalent non-Action input file is pp_generation_unconfined_constM.i
#
# A sample is constrained on all sides, except its top
# and its boundaries are
# also impermeable.  Fluid is pumped into the sample via a
# volumetric source (ie kg/second per cubic meter), and the
# rise in the top surface, porepressure, and stress are observed.
#
# In the standard poromechanics scenario, the Biot Modulus is held
# fixed and the source, s, has units m^3/second/m^3.  Then the expected result
# is
# strain_zz = disp_z = BiotCoefficient*BiotModulus*s*t/((bulk + 4*shear/3) + BiotCoefficient^2*BiotModulus)
# porepressure = BiotModulus*(s*t - BiotCoefficient*strain_zz)
# stress_xx = (bulk - 2*shear/3)*strain_zz   (remember this is effective stress)
# stress_zz = (bulk + 4*shear/3)*strain_zz   (remember this is effective stress)
#
# In porous_flow, however, the source has units kg/second/m^3.  The ratios remain
# fixed:
# stress_xx/strain_zz = (bulk - 2*shear/3) = 1 (for the parameters used here)
# stress_zz/strain_zz = (bulk + 4*shear/3) = 4 (for the parameters used here)
# porepressure/strain_zz = 13.3333333 (for the parameters used here)
#
# Expect
# disp_z = 0.3*10*s*t/((2 + 4*1.5/3) + 0.3^2*10) = 0.612245*s*t
# porepressure = 10*(s*t - 0.3*0.612245*s*t) = 8.163265*s*t
# stress_xx = (2 - 2*1.5/3)*0.612245*s*t = 0.612245*s*t
# stress_zz = (2 + 4*shear/3)*0.612245*s*t = 2.44898*s*t
# The relationship between the constant poroelastic source
# s (m^3/second/m^3) and the PorousFlow source, S (kg/second/m^3) is
# S = fluid_density * s = s * exp(porepressure/fluid_bulk)

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

[PorousFlowFullySaturated]
  porepressure = porepressure
  biot_coefficient = 0.3
  coupling_type = HydroMechanical
  displacements = 'disp_x disp_y disp_z'
  gravity = '0 0 0'
  fp = simple_fluid
  stabilization = Full
[]

[Kernels]
  [source]
    type = BodyForce
    function = '0.1*exp(8.163265306*0.1*t/3.3333333333)'
    variable = porepressure
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 3.3333333333
    density0 = 1
    thermal_expansion = 0
    viscosity = 1
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
    displacements = 'disp_x disp_y disp_z'
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [porosity]
    type = PorousFlowPorosityHMBiotModulus
    porosity_zero = 0.1
    biot_coefficient = 0.3
    solid_bulk = 2
    constant_fluid_bulk_modulus = 3.3333333333
    constant_biot_modulus = 10.0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0   0 1 0   0 0 1' # unimportant
  []
[]

[Postprocessors]
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
  file_base = pp_generation_unconfined_constM_action
  [csv]
    type = CSV
  []
[]
