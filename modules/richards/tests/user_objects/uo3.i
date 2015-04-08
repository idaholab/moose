# Seff User objects give the correct value
#
# If you want to add another test for another UserObject
# then add the UserObject, add a Function defining the expected result,
# add an AuxVariable and AuxKernel that will record the UserObject's value
# and finally add a NodalL2Error that compares this with the Function
#
# pressure = x (-5E6<=x<=5E6)

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
    value = x
  [../]

  [./answer_Seff1VG]
    type = ParsedFunction
    value = (1+max((-x)*al,0)^(1/(1-m)))^(-m)
    vars = 'al m'
    vals = '1E-6 0.8'
  [../]
  [./answer_dSeff1VG]
    type = GradParsedFunction
    direction = '1 0 0'
    value = (1+max((-x)*al,0)^(1/(1-m)))^(-m)
    vars = 'al m'
    vals = '1E-6 0.8'
  [../]
  [./answer_d2Seff1VG]
    type = Grad2ParsedFunction
    direction = '1 0 0'
    value = (1+max((-x)*al,0)^(1/(1-m)))^(-m)
    vars = 'al m'
    vals = '1E-6 0.8'
  [../]

  [./answer_Seff1BW]
    type = PiecewiseLinear
    format = columns
    data_file = satBW.csv
    axis = 0
  [../]
  [./answer_Seff1BWprime]
    type = PiecewiseLinear
    format = columns
    data_file = satBWprime.csv
    axis = 0
  [../]
  [./answer_Seff1BW2prime]
    type = PiecewiseLinear
    format = columns
    data_file = satBW2prime.csv
    axis = 0
  [../]

  [./answer_Seff1RSC]
    type = ParsedFunction
    value = (1+exp((-x-shift)/scale))^(-0.5)
    vars = 'shift scale'
    vals = '-2E6 1E6'
  [../]
  [./answer_dSeff1RSC]
    type = GradParsedFunction
    direction = '1 0 0'
    value = (1+exp((-x-shift)/scale))^(-0.5)
    vars = 'shift scale'
    vals = '-2E6 1E6'
  [../]
  [./answer_d2Seff1RSC]
    type = Grad2ParsedFunction
    direction = '1 0 0'
    value = (1+exp((-x-shift)/scale))^(-0.5)
    vars = 'shift scale'
    vals = '-2E6 1E6'
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

  [./check_AuxK]
    type = FunctionAux
    variable = check_Aux
    function = answer_Seff1RSC
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
[]



#############################################################################
#
# Following is largely unimportant as we're not running an actual similation
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
  active = 'csv'
  file_base = uo3
  print_perf_log = true
  [./csv]
    type = CSV
    interval = 1
  [../]
  [./exodus]
    type = Exodus
    hide = pressure
  [../]
[]
