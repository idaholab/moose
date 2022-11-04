# unsaturated = false
# gravity = true
# supg = true
# transient = true
# using RichardsMultiphaseProblem to bound pgas.  i take big timesteps to illustrate that the bounding works.  Note that s_res for gas = 0, in order to prevent the simulation from trying to reduce pgas at small x in order to conserve fluid mass by decreasing the density.  Because there is zero gas to begin with, but due to numerical inprecisions there is some gas at the end, the mass error for the gas is 0.5.

[Problem]
  type = RichardsMultiphaseProblem
  bounded_var = pgas
  lower_var = pwater
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmin = 0
  xmax = 1
[]


[GlobalParams]
  richardsVarNames_UO = PPNames
  density_UO = 'DensityWater DensityGas'
  relperm_UO = 'RelPermWater RelPermGas'
  SUPG_UO = 'SUPGwater SUPGgas'
  sat_UO = 'SatWater SatGas'
  seff_UO = 'SeffWater SeffGas'
  viscosity = '1E-3 0.5E-3'
  gravity = '-1 0 0'
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = 'pwater pgas'
  [../]
  [./DensityWater]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 1.0E2
  [../]
  [./DensityGas]
    type = RichardsDensityConstBulk
    dens0 = 0.5
    bulk_mod = 0.5E2
  [../]
  [./SeffWater]
    type = RichardsSeff2waterVG
    m = 0.8
    al = 1
  [../]
  [./SeffGas]
    type = RichardsSeff2gasVG
    m = 0.8
    al = 1
  [../]
  [./RelPermWater]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./RelPermGas]
    type = RichardsRelPermPower
    simm = 0.0
    n = 3
  [../]
  [./SatWater]
    type = RichardsSat
    s_res = 0.1
    sum_s_res = 0.1
  [../]
  [./SatGas]
    type = RichardsSat
    s_res = 0.00
    sum_s_res = 0.1
  [../]
  [./SUPGwater]
    type = RichardsSUPGstandard
    p_SUPG = 0.1
  [../]
  [./SUPGgas]
    type = RichardsSUPGstandard
    p_SUPG = 0.01
  [../]
[]

[Variables]
  [./pwater]
    order = FIRST
    family = LAGRANGE
  [../]
  [./pgas]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./water_ic]
    type = ConstantIC
    value = 1
    variable = pwater
  [../]
  [./gas_ic]
    type = ConstantIC
    value = 1
    variable = pgas
  [../]
[]


[Kernels]
  active = 'richardsfwater richardstwater richardsfgas richardstgas'
  [./richardstwater]
    type = RichardsLumpedMassChange
    variable = pwater
  [../]
  [./richardsfwater]
    type = RichardsFlux
    variable = pwater
  [../]
  [./richardstgas]
    type = RichardsLumpedMassChange
    variable = pgas
  [../]
  [./richardsfgas]
    type = RichardsFlux
    variable = pgas
  [../]
[]


[AuxVariables]
  [./seffgas]
  [../]
  [./seffwater]
  [../]

  # the following "dummy" variable is simply used for exception testing RichardsMultiphaseProblem
  # It is not part of the "gravity head" simulation
  [./dummy_var]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./seffgas_kernel]
    type = RichardsSeffAux
    pressure_vars = 'pwater pgas'
    seff_UO = SeffGas
    variable = seffgas
  [../]
  [./seffwater_kernel]
    type = RichardsSeffAux
    pressure_vars = 'pwater pgas'
    seff_UO = SeffWater
    variable = seffwater
  [../]
[]

[Postprocessors]
  [./mwater_init]
    type = RichardsMass
    variable = pwater
    execute_on = timestep_begin
    outputs = none
  [../]
  [./mgas_init]
    type = RichardsMass
    variable = pgas
    execute_on = timestep_begin
    outputs = none
  [../]
  [./mwater_fin]
    type = RichardsMass
    variable = pwater
    execute_on = timestep_end
    outputs = none
  [../]
  [./mgas_fin]
    type = RichardsMass
    variable = pgas
    execute_on = timestep_end
    outputs = none
  [../]

  [./mass_error_water]
    type = FunctionValuePostprocessor
    function = fcn_mass_error_w
  [../]

  [./pw_left]
    type = PointValue
    point = '0 0 0'
    variable = pwater
    outputs = none
  [../]
  [./pw_right]
    type = PointValue
    point = '1 0 0'
    variable = pwater
    outputs = none
  [../]
  [./error_water]
    type = FunctionValuePostprocessor
    function = fcn_error_water
  [../]
[]

[Functions]
  [./fcn_mass_error_w]
    type = ParsedFunction
    expression = 'abs(0.5*(mi-mf)/(mi+mf))'
    symbol_names = 'mi mf'
    symbol_values = 'mwater_init mwater_fin'
  [../]
  [./fcn_error_water]
    type = ParsedFunction
    expression = 'abs((-b*log(-(gdens0*xval+(-b*exp(-p0/b)))/b)-p1)/p1)'
    symbol_names = 'b gdens0 p0 xval p1'
    symbol_values = '1E2 -1 pw_left 1 pw_right'
  [../]
[]

[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
    linear_shape_fcns = true
  [../]
[]



[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-pc_factor_shift_type'
    petsc_options_value = 'nonzero'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1E6
  dt = 1E6
  dtmin = 1E6
  line_search = bt

  nl_rel_tol = 1.e-6
  nl_max_its = 10
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = gh_bounded_17
  csv = true
[]
