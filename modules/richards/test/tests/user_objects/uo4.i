# Seff 2-phase User objects give the correct value
#
# If you want to add another test for another UserObject
# then add the UserObject, add a Function defining the expected result,
# add an AuxVariable and AuxKernel that will record the UserObjects value
# and finally add a NodalL2Error that compares this with the Function
#
# Here pressure is x where x is between -5 and 5

[UserObjects]
  [./Seff2waterVG]
    type = RichardsSeff2waterVG
    m = 0.8
    al = 0.3
  [../]
  [./Seff2gasVG]
    type = RichardsSeff2gasVG
    m = 0.8
    al = 0.3
  [../]

  [./Seff2waterVGshifted]
    type = RichardsSeff2waterVGshifted
    m = 0.8
    al = 0.3
    shift = 2
  [../]
  [./Seff2gasVGshifted]
    type = RichardsSeff2gasVGshifted
    m = 0.8
    al = 0.3
    shift = 2
  [../]

  # following are unimportant in this test
  [./PPNames]
    type = RichardsVarNames
    richards_vars = 'pwater pgas'
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
    s_res = 0.05
    sum_s_res = 0.1
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 1
  [../]
[]

[Functions]
  [./initial_pwater]
    type = ParsedFunction
    expression = x
  [../]
  [./initial_pgas]
    type = ParsedFunction
    expression = 5.0
  [../]

  [./answer_Seff2waterVG]
    type = ParsedFunction
    expression = (1+max((-(x-5))*al,0)^(1/(1-m)))^(-m)
    symbol_names = 'al m'
    symbol_values = '0.3 0.8'
  [../]
  [./answer_dSeff2waterVG]
    type = GradParsedFunction
    direction = '1E-5 0 0'
    expression = (1+max((-(x-5))*al,0)^(1/(1-m)))^(-m)
    symbol_names = 'al m'
    symbol_values = '0.3 0.8'
  [../]
  [./answer_d2Seff2waterVG]
    type = Grad2ParsedFunction
    direction = '1E-4 0 0'
    expression = (1+max((-(x-5))*al,0)^(1/(1-m)))^(-m)
    symbol_names = 'al m'
    symbol_values = '0.3 0.8'
  [../]

  [./answer_Seff2gasVG]
    type = ParsedFunction
    expression = 1-(1+max((-(x-5))*al,0)^(1/(1-m)))^(-m)
    symbol_names = 'al m'
    symbol_values = '0.3 0.8'
  [../]
  [./answer_dSeff2gasVG]
    type = GradParsedFunction
    direction = '1E-5 0 0'
    expression = 1-(1+max((-(x-5))*al,0)^(1/(1-m)))^(-m)
    symbol_names = 'al m'
    symbol_values = '0.3 0.8'
  [../]
  [./answer_d2Seff2gasVG]
    type = Grad2ParsedFunction
    direction = '1E-4 0 0'
    expression = 1-(1+max((-(x-5))*al,0)^(1/(1-m)))^(-m)
    symbol_names = 'al m'
    symbol_values = '0.3 0.8'
  [../]

  [./answer_Seff2waterVGshifted]
    type = ParsedFunction
    expression = ((1+max((-(x-5-shift))*al,0)^(1/(1-m)))^(-m))/((1+max((-(-shift))*al,0)^(1/(1-m)))^(-m))
    symbol_names = 'al m shift'
    symbol_values = '0.3 0.8 2'
  [../]
  [./answer_dSeff2waterVGshifted]
    type = GradParsedFunction
    direction = '1E-5 0 0'
    expression = ((1+max((-(x-5-shift))*al,0)^(1/(1-m)))^(-m))/((1+max((-(-shift))*al,0)^(1/(1-m)))^(-m))
    symbol_names = 'al m shift'
    symbol_values = '0.3 0.8 2'
  [../]
  [./answer_d2Seff2waterVGshifted]
    type = Grad2ParsedFunction
    direction = '1E-4 0 0'
    expression = ((1+max((-(x-5-shift))*al,0)^(1/(1-m)))^(-m))/((1+max((-(-shift))*al,0)^(1/(1-m)))^(-m))
    symbol_names = 'al m shift'
    symbol_values = '0.3 0.8 2'
  [../]

  [./answer_Seff2gasVGshifted]
    type = ParsedFunction
    expression = 1-((1+max((-(x-5-shift))*al,0)^(1/(1-m)))^(-m))/((1+max((-(-shift))*al,0)^(1/(1-m)))^(-m))
    symbol_names = 'al m shift'
    symbol_values = '0.3 0.8 2'
  [../]
  [./answer_dSeff2gasVGshifted]
    type = GradParsedFunction
    direction = '1E-5 0 0'
    expression = 1-((1+max((-(x-5-shift))*al,0)^(1/(1-m)))^(-m))/((1+max((-(-shift))*al,0)^(1/(1-m)))^(-m))
    symbol_names = 'al m shift'
    symbol_values = '0.3 0.8 2'
  [../]
  [./answer_d2Seff2gasVGshifted]
    type = Grad2ParsedFunction
    direction = '1E-4 0 0'
    expression = 1-((1+max((-(x-5-shift))*al,0)^(1/(1-m)))^(-m))/((1+max((-(-shift))*al,0)^(1/(1-m)))^(-m))
    symbol_names = 'al m shift'
    symbol_values = '0.3 0.8 2'
  [../]
[]

[AuxVariables]
  [./Seff2waterVG_Aux]
  [../]
  [./dSeff2waterVG_Aux]
  [../]
  [./d2Seff2waterVG_Aux]
  [../]
  [./Seff2gasVG_Aux]
  [../]
  [./dSeff2gasVG_Aux]
  [../]
  [./d2Seff2gasVG_Aux]
  [../]

  [./Seff2waterVGshifted_Aux]
  [../]
  [./dSeff2waterVGshifted_Aux]
  [../]
  [./d2Seff2waterVGshifted_Aux]
  [../]
  [./Seff2gasVGshifted_Aux]
  [../]
  [./dSeff2gasVGshifted_Aux]
  [../]
  [./d2Seff2gasVGshifted_Aux]
  [../]

  [./check_Aux]
  [../]
[]

[AuxKernels]
  [./Seff2waterVG_AuxK]
    type = RichardsSeffAux
    variable = Seff2waterVG_Aux
    seff_UO = Seff2waterVG
    pressure_vars = 'pwater pgas'
  [../]
  [./dSeff2waterVG_AuxK]
    type = RichardsSeffPrimeAux
    variable = dSeff2waterVG_Aux
    seff_UO = Seff2waterVG
    pressure_vars = 'pwater pgas'
    wrtnum = 0
  [../]
  [./d2Seff2waterVG_AuxK]
    type = RichardsSeffPrimePrimeAux
    variable = d2Seff2waterVG_Aux
    seff_UO = Seff2waterVG
    pressure_vars = 'pwater pgas'
    wrtnum1 = 0
    wrtnum2 = 0
  [../]
  [./Seff2gasVG_AuxK]
    type = RichardsSeffAux
    variable = Seff2gasVG_Aux
    seff_UO = Seff2gasVG
    pressure_vars = 'pwater pgas'
  [../]
  [./dSeff2gasVG_AuxK]
    type = RichardsSeffPrimeAux
    variable = dSeff2gasVG_Aux
    seff_UO = Seff2gasVG
    pressure_vars = 'pwater pgas'
    wrtnum = 0
  [../]
  [./d2Seff2gasVG_AuxK]
    type = RichardsSeffPrimePrimeAux
    variable = d2Seff2gasVG_Aux
    seff_UO = Seff2gasVG
    pressure_vars = 'pwater pgas'
    wrtnum1 = 0
    wrtnum2 = 0
  [../]

  [./Seff2waterVGshifted_AuxK]
    type = RichardsSeffAux
    variable = Seff2waterVGshifted_Aux
    seff_UO = Seff2waterVGshifted
    pressure_vars = 'pwater pgas'
  [../]
  [./dSeff2waterVGshifted_AuxK]
    type = RichardsSeffPrimeAux
    variable = dSeff2waterVGshifted_Aux
    seff_UO = Seff2waterVGshifted
    pressure_vars = 'pwater pgas'
    wrtnum = 0
  [../]
  [./d2Seff2waterVGshifted_AuxK]
    type = RichardsSeffPrimePrimeAux
    variable = d2Seff2waterVGshifted_Aux
    seff_UO = Seff2waterVGshifted
    pressure_vars = 'pwater pgas'
    wrtnum1 = 0
    wrtnum2 = 0
  [../]
  [./Seff2gasVGshifted_AuxK]
    type = RichardsSeffAux
    variable = Seff2gasVGshifted_Aux
    seff_UO = Seff2gasVGshifted
    pressure_vars = 'pwater pgas'
  [../]
  [./dSeff2gasVGshifted_AuxK]
    type = RichardsSeffPrimeAux
    variable = dSeff2gasVGshifted_Aux
    seff_UO = Seff2gasVGshifted
    pressure_vars = 'pwater pgas'
    wrtnum = 0
  [../]
  [./d2Seff2gasVGshifted_AuxK]
    type = RichardsSeffPrimePrimeAux
    variable = d2Seff2gasVGshifted_Aux
    seff_UO = Seff2gasVGshifted
    pressure_vars = 'pwater pgas'
    wrtnum1 = 0
    wrtnum2 = 0
  [../]

  [./check_AuxK]
    type = FunctionAux
    variable = check_Aux
    function = answer_d2Seff2waterVGshifted
  [../]
[]

[Postprocessors]
  [./cf_Seff2waterVG]
    type = NodalL2Error
    function = answer_Seff2waterVG
    variable = Seff2waterVG_Aux
  [../]
  [./cf_dSeff2waterVG]
    type = NodalL2Error
    function = answer_dSeff2waterVG
    variable = dSeff2waterVG_Aux
  [../]
  [./cf_d2Seff2waterVG]
    type = NodalL2Error
    function = answer_d2Seff2waterVG
    variable = d2Seff2waterVG_Aux
  [../]

  [./cf_Seff2gasVG]
    type = NodalL2Error
    function = answer_Seff2gasVG
    variable = Seff2gasVG_Aux
  [../]
  [./cf_dSeff2gasVG]
    type = NodalL2Error
    function = answer_dSeff2gasVG
    variable = dSeff2gasVG_Aux
  [../]
  [./cf_d2Seff2gasVG]
    type = NodalL2Error
    function = answer_d2Seff2gasVG
    variable = d2Seff2gasVG_Aux
  [../]

  [./cf_Seff2waterVGshifted]
    type = NodalL2Error
    function = answer_Seff2waterVGshifted
    variable = Seff2waterVGshifted_Aux
  [../]
  [./cf_dSeff2waterVGshifted]
    type = NodalL2Error
    function = answer_dSeff2waterVGshifted
    variable = dSeff2waterVGshifted_Aux
  [../]
  [./cf_d2Seff2waterVGshifted]
    type = NodalL2Error
    function = answer_d2Seff2waterVGshifted
    variable = d2Seff2waterVGshifted_Aux
  [../]

  [./cf_Seff2gasVGshifted]
    type = NodalL2Error
    function = answer_Seff2gasVGshifted
    variable = Seff2gasVGshifted_Aux
  [../]
  [./cf_dSeff2gasVGshifted]
    type = NodalL2Error
    function = answer_dSeff2gasVGshifted
    variable = dSeff2gasVGshifted_Aux
  [../]
  [./cf_d2Seff2gasVGshifted]
    type = NodalL2Error
    function = answer_d2Seff2gasVGshifted
    variable = d2Seff2gasVGshifted_Aux
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
  xmin = -5
  xmax = 5
[]

[Variables]
  [./pwater]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = initial_pwater
    [../]
  [../]
  [./pgas]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = initial_pgas
    [../]
  [../]
[]

[Kernels]
  active = 'watert gast'
  [./watert]
    type = RichardsMassChange
    richardsVarNames_UO = PPNames
    variable = pwater
  [../]
  [./gast]
    type = RichardsMassChange
    richardsVarNames_UO = PPNames
    variable = pgas
  [../]
[]

[Materials]
  [./unimportant_material]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-20 0 0  0 1E-20 0  0 0 1E-20'
    richardsVarNames_UO = PPNames
    density_UO = 'DensityConstBulk DensityConstBulk'
    relperm_UO = 'RelPermPower RelPermPower'
    sat_UO = 'Saturation Saturation'
    seff_UO = 'Seff2waterVG Seff2gasVG'
    SUPG_UO = 'SUPGstandard SUPGstandard'
    viscosity = '1E-3 1E-5'
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
  file_base = uo4
  [./csv]
    type = CSV
    [../]
  [./exodus]
    type = Exodus
  [../]
[]
