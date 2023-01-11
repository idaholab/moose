# with immobile saturation - this illustrates a perfect case of fullyupwind working very well
# unsaturated = true
# gravity = true
# full upwinding = true
# transient = true

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
[]

[Functions]
  [./dts]
    type = PiecewiseLinear
    y = '1E-2 1E-1 1E0 0.5E1 0.5E2 0.4E4 1E5 1E6 1E7'
    x = '0 1E-1 1E0 1E1 1E2 1E3 1E4 1E5 1E6'
  [../]
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
    simm = 0.4
    n = 2
  [../]
  [./RelPermGas]
    type = RichardsRelPermPower
    simm = 0.3
    n = 2
  [../]
  [./SatWater]
    type = RichardsSat
    s_res = 0.1
    sum_s_res = 0.15
  [../]
  [./SatGas]
    type = RichardsSat
    s_res = 0.05
    sum_s_res = 0.15
  [../]
  [./SUPGwater]
    type = RichardsSUPGstandard
    p_SUPG = 1E-5
  [../]
  [./SUPGgas]
    type = RichardsSUPGstandard
    p_SUPG = 1E-5
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
    value = 2
    variable = pgas
  [../]
[]


[Kernels]
  active = 'richardsfwater richardstwater richardsfgas richardstgas'
  [./richardstwater]
    type = RichardsMassChange
    variable = pwater
  [../]
  [./richardsfwater]
    type = RichardsFullyUpwindFlux
    variable = pwater
  [../]
  [./richardstgas]
    type = RichardsMassChange
    variable = pgas
  [../]
  [./richardsfgas]
    type = RichardsFullyUpwindFlux
    variable = pgas
  [../]
[]


[AuxVariables]
  [./seffgas]
  [../]
  [./seffwater]
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
  [./mass_error_gas]
    type = FunctionValuePostprocessor
    function = fcn_mass_error_g
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

  [./pg_left]
    type = PointValue
    point = '0 0 0'
    variable = pgas
    outputs = none
  [../]
  [./pg_right]
    type = PointValue
    point = '1 0 0'
    variable = pgas
    outputs = none
  [../]
  [./error_gas]
    type = FunctionValuePostprocessor
    function = fcn_error_gas
  [../]
[]

[Functions]
  [./fcn_mass_error_w]
    type = ParsedFunction
    expression = 'abs(0.5*(mi-mf)/(mi+mf))'
    symbol_names = 'mi mf'
    symbol_values = 'mwater_init mwater_fin'
  [../]
  [./fcn_mass_error_g]
    type = ParsedFunction
    expression = 'abs(0.5*(mi-mf)/(mi+mf))'
    symbol_names = 'mi mf'
    symbol_values = 'mgas_init mgas_fin'
  [../]
  [./fcn_error_water]
    type = ParsedFunction
    expression = 'abs((-b*log(-(gdens0*xval+(-b*exp(-p0/b)))/b)-p1)/p1)'
    symbol_names = 'b gdens0 p0 xval p1'
    symbol_values = '1E2 -1 pw_left 1 pw_right'
  [../]
  [./fcn_error_gas]
    type = ParsedFunction
    expression = 'abs((-b*log(-(gdens0*xval+(-b*exp(-p0/b)))/b)-p1)/p1)'
    symbol_names = 'b gdens0 p0 xval p1'
    symbol_values = '0.5E2 -0.5 pg_left 1 pg_right'
  [../]
[]

[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
    viscosity = '1E-3 0.5E-3'
    gravity = '-1 0 0'
    linear_shape_fcns = true
  [../]
[]



[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1E6

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]

[Outputs]
  file_base = gh_fu_18
  execute_on = 'timestep_end final'
  interval = 100000
  exodus = true
[]
