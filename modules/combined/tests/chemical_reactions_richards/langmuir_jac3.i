# testing whether when we have a centre block containing 'conc' which is a CONSTANT MONOMIAL, and two-phase Richards flow, we get the correct Jacobian
[Mesh]
  type = FileMesh
  file = three_eles.e
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
  density_UO = 'DensityWater DensityGas'
  relperm_UO = 'RelPermWater RelPermGas'
  SUPG_UO = 'SUPGstandard SUPGstandard'
  sat_UO = 'Saturation Saturation'
  seff_UO = 'SeffWater SeffGas'
  viscosity = '1E-3 1.1E-5'
  gravity = '0 0 -10'
  linear_shape_fcns = true
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = 'pwater pgas'
  [../]
  [./DensityWater]
    type = RichardsDensityConstBulk
    dens0 = 1000
    bulk_mod = 2E9
  [../]
  [./DensityGas]
    type = RichardsDensityMethane20degC
  [../]
  [./SeffWater]
    type = RichardsSeff2waterVG
    m = 0.8
    al = 1E-5
  [../]
  [./SeffGas]
    type = RichardsSeff2gasVG
    m = 0.8
    al = 1E-5
  [../]
  [./RelPermWater]
    type = RichardsRelPermPower
    simm = 0.2
    n = 3
  [../]
  [./RelPermGas]
    type = RichardsRelPermPower
    simm = 0.0
    n = 3
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0.0
    sum_s_res = 0.0
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 1.0E+1
  [../]
[]

[Variables]
  [./pwater]
  [../]
  [./pgas]
  [../]
  [./conc]
    family = MONOMIAL
    order = CONSTANT
    block = centre_block
  [../]
[]

[ICs]
  [./water]
    type = ConstantIC
    variable = pwater
    value = 0.0
  [../]
  [./gas]
    type = RandomIC
    variable = pgas
    min = 0
    max = 5E5
  [../]
  [./conc_ic]
    type = RandomIC
    variable = conc
    min = 0
    max = 20
    block = centre_block
  [../]
[]


[Kernels]
  [./richardstwater]
    type = RichardsMassChange
    variable = pwater
  [../]
  [./richardsfwater]
    type = RichardsFlux
    variable = pwater
  [../]
  [./richardstgas]
    type = RichardsMassChange
    variable = pgas
  [../]
  [./richardsfgas]
    type = RichardsFlux
    variable = pgas
  [../]
  [./c_dot]
    type = TimeDerivative
    block = centre_block
    variable = conc
  [../]
  [./flow_from_matrix]
    type = DesorptionFromMatrix
    block = centre_block
    variable = conc
    pressure_var = pgas
  [../]
  [./flux_to_porespace]
    type = DesorptionToPorespace
    block = centre_block
    variable = pgas
    conc_var = conc
  [../]
[]

[Materials]
  [./all_blocks]
    type = RichardsMaterial
    block = 'left_block centre_block right_block'
    mat_porosity = 0.02
    mat_permeability = '1E-15 0 0  0 1E-15 0  0 0 1E-16'
  [../]
  [./langmuir_params]
    type = LangmuirMaterial
    block = centre_block
    one_over_desorption_time_const = 0.813
    one_over_adsorption_time_const = 0.813
    langmuir_density = 20.0
    langmuir_pressure = 1.5E6
    pressure_var = pgas
    conc_var = conc
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    #petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E3 # get rid of the large c_dot contribution
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = langmuir_jac3
[]
