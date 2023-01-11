# Seff User objects give the correct value
# Sat User objects give the correct value
#
# If you want to add another test for another UserObject
# then add the UserObject, add a Function defining the expected result,
# add an AuxVariable and AuxKernel that will record the UserObjects value
# and finally add a NodalL2Error that compares this with the Function
#
# Here pressure is x where x runs between -5E6 and 5E6

[UserObjects]
  [./Seff1VG]
    type = RichardsSeff1VG
    m = 0.8
    al = 1E-6
  [../]
  [./Seff1BWsmall]
    type = RichardsSeff1BWsmall
    Sn = 0.0
    Ss = 1.0
    C = 1.01
    las = 1E5
  [../]
  [./Seff1RSC]
    type = RichardsSeff1RSC
    oil_viscosity = 4.0
    scale_ratio = 1E6
    shift = -2E6
  [../]
  [./Seff1VGcut]
    type = RichardsSeff1VGcut
    m = 0.8
    al = 1E-6
    p_cut = -1E6
  [../]

  # following are unimportant in this test
  [./PPNames]
    type = RichardsVarNames
    richards_vars = pressure
  [../]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1000
    bulk_mod = 2E6
  [../]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.10101
    n = 2
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0.054321
    sum_s_res = 0.054321
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 1E5
  [../]
[]

[Functions]
  [./initial_pressure]
    type = ParsedFunction
    expression = x
  [../]

  [./answer_Seff1VG]
    type = ParsedFunction
    expression = (1+max((-x)*al,0)^(1/(1-m)))^(-m)
    symbol_names = 'al m'
    symbol_values = '1E-6 0.8'
  [../]
  [./answer_dSeff1VG]
    type = GradParsedFunction
    direction = '1 0 0'
    expression = (1+max((-x)*al,0)^(1/(1-m)))^(-m)
    symbol_names = 'al m'
    symbol_values = '1E-6 0.8'
  [../]
  [./answer_d2Seff1VG]
    type = Grad2ParsedFunction
    direction = '1 0 0'
    expression = (1+max((-x)*al,0)^(1/(1-m)))^(-m)
    symbol_names = 'al m'
    symbol_values = '1E-6 0.8'
  [../]

  [./answer_Seff1BW]
    type = PiecewiseLinear
    format = columns
    data_file = satBW.csv
    axis = x
  [../]
  [./answer_Seff1BWprime]
    type = PiecewiseLinear
    format = columns
    data_file = satBWprime.csv
    axis = x
  [../]
  [./answer_Seff1BW2prime]
    type = PiecewiseLinear
    format = columns
    data_file = satBW2prime.csv
    axis = x
  [../]

  [./answer_Seff1RSC]
    type = ParsedFunction
    expression = (1+exp((-x-shift)/scale))^(-0.5)
    symbol_names = 'shift scale'
    symbol_values = '-2E6 1E6'
  [../]
  [./answer_dSeff1RSC]
    type = GradParsedFunction
    direction = '1 0 0'
    expression = (1+exp((-x-shift)/scale))^(-0.5)
    symbol_names = 'shift scale'
    symbol_values = '-2E6 1E6'
  [../]
  [./answer_d2Seff1RSC]
    type = Grad2ParsedFunction
    direction = '1 0 0'
    expression = (1+exp((-x-shift)/scale))^(-0.5)
    symbol_names = 'shift scale'
    symbol_values = '-2E6 1E6'
  [../]

  [./answer_Seff1VGcut]
    type = ParsedFunction
    expression = if(x<pcut,scut+dscut*(x-pcut),(1+max((-x)*al,0)^(1/(1-m)))^(-m))
    symbol_names = 'al m pcut scut dscut'
    symbol_values = '1E-6 0.8 -1E6 0.574349177498517 1.14869835499703e-06'
  [../]
  [./answer_dSeff1VGcut]
    type = GradParsedFunction
    direction = '1 0 0'
    expression = if(x<pcut,scut+dscut*(x-pcut),(1+max((-x)*al,0)^(1/(1-m)))^(-m))
    symbol_names = 'al m pcut scut dscut'
    symbol_values = '1E-6 0.8 -1E6 0.574349177498517 1.14869835499703e-06'
  [../]
  [./answer_d2Seff1VGcut]
    type = Grad2ParsedFunction
    direction = '1 0 0'
    expression = if(x<pcut,scut+dscut*(x-pcut),(1+max((-x)*al,0)^(1/(1-m)))^(-m))
    symbol_names = 'al m pcut scut dscut'
    symbol_values = '1E-6 0.8 -1E6 0.574349177498517 1.14869835499703e-06'
  [../]

  [./answer_Sat]
    type = ParsedFunction
    expression = sres+((1-sumsres)*((1+max((-x)*al,0)^(1/(1-m)))^(-m)))
    symbol_names = 'al m sres sumsres'
    symbol_values = '1E-6 0.8 0.054321 0.054321'
  [../]
  [./answer_dSat]
    type = ParsedFunction
    expression = 1-sumsres
    symbol_names = 'sumsres'
    symbol_values = '0.054321'
  [../]
[]

[AuxVariables]
  [./Seff1VG_Aux]
  [../]
  [./dSeff1VG_Aux]
  [../]
  [./d2Seff1VG_Aux]
  [../]

  [./Seff1BWsmall_Aux]
  [../]
  [./dSeff1BWsmall_Aux]
  [../]
  [./d2Seff1BWsmall_Aux]
  [../]

  [./Seff1RSC_Aux]
  [../]
  [./dSeff1RSC_Aux]
  [../]
  [./d2Seff1RSC_Aux]
  [../]

  [./Seff1VGcut_Aux]
  [../]
  [./dSeff1VGcut_Aux]
  [../]
  [./d2Seff1VGcut_Aux]
  [../]

  [./Sat_Aux]
  [../]
  [./dSat_Aux]
  [../]

  [./check_Aux]
  [../]
[]

[AuxKernels]
  [./Seff1VG_AuxK]
    type = RichardsSeffAux
    variable = Seff1VG_Aux
    seff_UO = Seff1VG
    pressure_vars = pressure
  [../]
  [./dSeff1VG_AuxK]
    type = RichardsSeffPrimeAux
    variable = dSeff1VG_Aux
    seff_UO = Seff1VG
    pressure_vars = pressure
    wrtnum = 0
  [../]
  [./d2Seff1VG_AuxK]
    type = RichardsSeffPrimePrimeAux
    variable = d2Seff1VG_Aux
    seff_UO = Seff1VG
    pressure_vars = pressure
    wrtnum1 = 0
    wrtnum2 = 0
  [../]

  [./Seff1BWsmall_AuxK]
    type = RichardsSeffAux
    variable = Seff1BWsmall_Aux
    seff_UO = Seff1BWsmall
    pressure_vars = pressure
  [../]
  [./dSeff1BWsmall_AuxK]
    type = RichardsSeffPrimeAux
    variable = dSeff1BWsmall_Aux
    seff_UO = Seff1BWsmall
    pressure_vars = pressure
    wrtnum = 0
  [../]
  [./d2Seff1BWsmall_AuxK]
    type = RichardsSeffPrimePrimeAux
    variable = d2Seff1BWsmall_Aux
    seff_UO = Seff1BWsmall
    pressure_vars = pressure
    wrtnum1 = 0
    wrtnum2 = 0
  [../]

  [./Seff1RSC_AuxK]
    type = RichardsSeffAux
    variable = Seff1RSC_Aux
    seff_UO = Seff1RSC
    pressure_vars = pressure
  [../]
  [./dSeff1RSC_AuxK]
    type = RichardsSeffPrimeAux
    variable = dSeff1RSC_Aux
    seff_UO = Seff1RSC
    pressure_vars = pressure
    wrtnum = 0
  [../]
  [./d2Seff1RSC_AuxK]
    type = RichardsSeffPrimePrimeAux
    variable = d2Seff1RSC_Aux
    seff_UO = Seff1RSC
    pressure_vars = pressure
    wrtnum1 = 0
    wrtnum2 = 0
  [../]

  [./Seff1VGcut_AuxK]
    type = RichardsSeffAux
    variable = Seff1VGcut_Aux
    seff_UO = Seff1VGcut
    pressure_vars = pressure
  [../]
  [./dSeff1VGcut_AuxK]
    type = RichardsSeffPrimeAux
    variable = dSeff1VGcut_Aux
    seff_UO = Seff1VGcut
    pressure_vars = pressure
    wrtnum = 0
  [../]
  [./d2Seff1VGcut_AuxK]
    type = RichardsSeffPrimePrimeAux
    variable = d2Seff1VGcut_Aux
    seff_UO = Seff1VGcut
    pressure_vars = pressure
    wrtnum1 = 0
    wrtnum2 = 0
  [../]

  [./Sat_AuxK]
    type = RichardsSatAux
    sat_UO = Saturation
    seff_var = Seff1VG_Aux
    variable = Sat_Aux
  [../]
  [./dSat_AuxK]
    type = RichardsSatPrimeAux
    sat_UO = Saturation
    seff_var = Seff1VG_Aux
    variable = dSat_Aux
  [../]

  [./check_AuxK]
    type = FunctionAux
    variable = check_Aux
    function = answer_Seff1VGcut
  [../]
[]

[Postprocessors]
  [./cf_Seff1VG]
    type = NodalL2Error
    function = answer_Seff1VG
    variable = Seff1VG_Aux
  [../]
  [./cf_dSeff1VG]
    type = NodalL2Error
    function = answer_dSeff1VG
    variable = dSeff1VG_Aux
  [../]
  [./cf_d2Seff1VG]
    type = NodalL2Error
    function = answer_d2Seff1VG
    variable = d2Seff1VG_Aux
  [../]

  [./cf_Seff1BW]
    type = NodalL2Error
    function = answer_Seff1BW
    variable = Seff1BWsmall_Aux
  [../]
  [./cf_Seff1BWprime]
    type = NodalL2Error
    function = answer_Seff1BWprime
    variable = dSeff1BWsmall_Aux
  [../]
  [./cf_Seff1BW2prime]
    type = NodalL2Error
    function = answer_Seff1BW2prime
    variable = d2Seff1BWsmall_Aux
  [../]

  [./cf_Seff1RSC]
    type = NodalL2Error
    function = answer_Seff1RSC
    variable = Seff1RSC_Aux
  [../]
  [./cf_dSeff1RSC]
    type = NodalL2Error
    function = answer_dSeff1RSC
    variable = dSeff1RSC_Aux
  [../]
  [./cf_d2Seff1RSC]
    type = NodalL2Error
    function = answer_d2Seff1RSC
    variable = d2Seff1RSC_Aux
  [../]

  [./cf_Seff1VGcut]
    type = NodalL2Error
    function = answer_Seff1VGcut
    variable = Seff1VGcut_Aux
  [../]
  [./cf_dSeff1VGcut]
    type = NodalL2Error
    function = answer_dSeff1VGcut
    variable = dSeff1VGcut_Aux
  [../]
  [./cf_d2Seff1VGcut]
    type = NodalL2Error
    function = answer_d2Seff1VGcut
    variable = d2Seff1VGcut_Aux
  [../]

  [./cf_Sat]
    type = NodalL2Error
    function = answer_Sat
    variable = Sat_Aux
  [../]
  [./cf_dSat]
    type = NodalL2Error
    function = answer_dSat
    variable = dSat_Aux
  [../]
[]



#############################################################################
#
# Following is largely unimportant as we are not running an actual similation
#
#############################################################################
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = -5E6
  xmax = 5E6
[]

[Variables]
  [./pressure]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = initial_pressure
    [../]
  [../]
[]

[Kernels]
  active = 'richardsf richardst'
  [./richardst]
    type = RichardsMassChange
    richardsVarNames_UO = PPNames
    variable = pressure
  [../]
  [./richardsf]
    type = RichardsFlux
    richardsVarNames_UO = PPNames
    variable = pressure
  [../]
[]

[Materials]
  [./unimportant_material]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-20 0 0  0 1E-20 0  0 0 1E-20'
    richardsVarNames_UO = PPNames
    density_UO = DensityConstBulk
    relperm_UO = RelPermPower
    sat_UO = Saturation
    seff_UO = Seff1VG
    SUPG_UO = SUPGstandard
    viscosity = 1E-3
    gravity = '0 0 -10'
    linear_shape_fcns = true
  [../]
[]

[Preconditioning]
  [./does_nothing]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E50 1E50 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  num_steps = 1
  dt = 1E-100
[]

[Outputs]
  execute_on = 'timestep_end'
  active = 'csv'
  file_base = uo3
  [./csv]
    type = CSV
  [../]
  [./exodus]
    type = Exodus
    hide = pressure
  [../]
[]
