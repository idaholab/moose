# Relative-permeability User objects give the correct value
# (note that here p is x, where x runs between 0.01 and 0.99
# and that seff is p in the aux vars)
#
# If you want to add another test for another UserObject
# then add the UserObject, add a Function defining the expected result,
# add an AuxVariable and AuxKernel that will record the UserObjects value
# and finally add a NodalL2Error that compares this with the Function.

[UserObjects]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./RelPermPower5]
    type = RichardsRelPermPower
    simm = 0.0
    n = 5
  [../]
  [./RelPermVG]
    type = RichardsRelPermVG
    simm = 0.0
    m = 0.8
  [../]
  [./RelPermVG1]
    type = RichardsRelPermVG1
    simm = 0.0
    m = 0.8
    scut = 1E-6 # then we get a cubic
  [../]
  [./RelPermBW]
    type = RichardsRelPermBW
    Sn = 0.05
    Ss = 0.95
    Kn = 0.0
    Ks = 1.0
    C = 1.5
  [../]
  [./RelPermMonomial]
    type = RichardsRelPermMonomial
    simm = 0.0
    n = 3
  [../]
  [./RelPermPowerGas]
    type = RichardsRelPermPowerGas
    simm = 0.0
    n = 5
  [../]
  [./Q2PRelPermPowerGas]
    type = Q2PRelPermPowerGas
    simm = 0.0
    n = 5
  [../]
  [./RelPermMonomial_zero]
    type = RichardsRelPermMonomial
    simm = 0.1
    n = 0
    zero_to_the_zero = 0
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
  [./SeffVG]
    type = RichardsSeff1VG
    m = 0.8
    al = 1E-6
  [../]
  [./RelPermPower_unimportant]
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

  [./answer_RelPermPower]
    type = ParsedFunction
    expression = ((n+1)*(x^n))-(n*(x^(n+1)))
    symbol_names = 'n'
    symbol_values = '2'
  [../]
  [./answer_dRelPermPower]
    type = GradParsedFunction
    direction = '1E-4 0 0'
    expression = ((n+1)*(x^n))-(n*(x^(n+1)))
    symbol_names = 'n'
    symbol_values = '2'
  [../]
  [./answer_d2RelPermPower]
    type = Grad2ParsedFunction
    direction = '1E-3 0 0'
    expression = ((n+1)*(x^n))-(n*(x^(n+1)))
    symbol_names = 'n'
    symbol_values = '2'
  [../]

  [./answer_RelPermPower5]
    type = ParsedFunction
    expression = ((n+1)*(x^n))-(n*(x^(n+1)))
    symbol_names = 'n'
    symbol_values = '5'
  [../]
  [./answer_dRelPermPower5]
    type = GradParsedFunction
    direction = '1E-4 0 0'
    expression = ((n+1)*(x^n))-(n*(x^(n+1)))
    symbol_names = 'n'
    symbol_values = '5'
  [../]
  [./answer_d2RelPermPower5]
    type = Grad2ParsedFunction
    direction = '1E-5 0 0'
    expression = ((n+1)*(x^n))-(n*(x^(n+1)))
    symbol_names = 'n'
    symbol_values = '5'
  [../]

  [./answer_RelPermVG]
    type = ParsedFunction
    expression = (x^(0.5))*(1-(1-(x^(1.0/m)))^m)^2
    symbol_names = 'm'
    symbol_values = '0.8'
  [../]
  [./answer_dRelPermVG]
    type = GradParsedFunction
    direction = '1E-4 0 0'
    expression = (x^(0.5))*(1-(1-(x^(1.0/m)))^m)^2
    symbol_names = 'm'
    symbol_values = '0.8'
  [../]
  [./answer_d2RelPermVG]
    type = Grad2ParsedFunction
    direction = '1E-5 0 0'
    expression = (x^(0.5))*(1-(1-(x^(1.0/m)))^m)^2
    symbol_names = 'm'
    symbol_values = '0.8'
  [../]

  [./answer_RelPermVG1]
    type = ParsedFunction
    expression = x^3
  [../]
  [./answer_dRelPermVG1]
    type = GradParsedFunction
    direction = '1E-4 0 0'
    expression = x^3
  [../]
  [./answer_d2RelPermVG1]
    type = Grad2ParsedFunction
    direction = '1E-5 0 0'
    expression = x^3
  [../]

  [./answer_RelPermBW]
    type = ParsedFunction
    expression = if(x>ss,1,if(x<sn,0,kn+(((x-sn)/(ss-sn))^2)*(c-1)*(ks-kn)/(c-((x-sn)/(ss-sn)))))
    symbol_names = 'kn ks c sn ss'
    symbol_values = '0 1 1.5 0.05 0.95'
  [../]
  [./answer_dRelPermBW]
    type = GradParsedFunction
    direction = '1E-4 0 0'
    expression = if(x>ss,1,if(x<sn,0,kn+(((x-sn)/(ss-sn))^2)*(c-1)*(ks-kn)/(c-((x-sn)/(ss-sn)))))
    symbol_names = 'kn ks c sn ss'
    symbol_values = '0 1 1.5 0.05 0.95'
  [../]
  [./answer_d2RelPermBW]
    type = Grad2ParsedFunction
    direction = '1E-5 0 0'
    expression = if(x>ss,1,if(x<sn,0,kn+(((x-sn)/(ss-sn))^2)*(c-1)*(ks-kn)/(c-((x-sn)/(ss-sn)))))
    symbol_names = 'kn ks c sn ss'
    symbol_values = '0 1 1.5 0.05 0.95'
  [../]

  [./answer_RelPermMonomial]
    type = ParsedFunction
    expression = x^n
    symbol_names = 'n'
    symbol_values = '3'
  [../]
  [./answer_dRelPermMonomial]
    type = GradParsedFunction
    direction = '1E-4 0 0'
    expression = x^n
    symbol_names = 'n'
    symbol_values = '3'
  [../]
  [./answer_d2RelPermMonomial]
    type = Grad2ParsedFunction
    direction = '1E-3 0 0'
    expression = x^n
    symbol_names = 'n'
    symbol_values = '3'
  [../]

  [./answer_RelPermMonomial_zero]
    type = ParsedFunction
    expression = if(x>simm,1,0)
    symbol_names = 'simm'
    symbol_values = '0.1'
  [../]
  [./answer_dRelPermMonomial_zero]
    type = GradParsedFunction
    direction = '1E-4 0 0'
    expression = if(x>simm,1,0)
    symbol_names = 'simm'
    symbol_values = '0.1'
  [../]
  [./answer_d2RelPermMonomial_zero]
    type = Grad2ParsedFunction
    direction = '1E-3 0 0'
    expression = if(x>simm,1,0)
    symbol_names = 'simm'
    symbol_values = '0.1'
  [../]

  [./answer_RelPermPowerGas]
    type = ParsedFunction
    expression = 1-((n+1)*((1-x)^n))+(n*((1-x)^(n+1)))
    symbol_names = 'n'
    symbol_values = '5'
  [../]
  [./answer_dRelPermPowerGas]
    type = GradParsedFunction
    direction = '1E-4 0 0'
    expression = 1-((n+1)*((1-x)^n))+(n*((1-x)^(n+1)))
    symbol_names = 'n'
    symbol_values = '5'
  [../]
  [./answer_d2RelPermPowerGas]
    type = Grad2ParsedFunction
    direction = '1E-5 0 0'
    expression = 1-((n+1)*((1-x)^n))+(n*((1-x)^(n+1)))
    symbol_names = 'n'
    symbol_values = '5'
  [../]

  [./answer_Q2PRelPermPowerGas]
    type = ParsedFunction
    expression = 1-((n+1)*(x^n))+(n*(x^(n+1)))
    symbol_names = 'n'
    symbol_values = '5'
  [../]
  [./answer_dQ2PRelPermPowerGas]
    type = GradParsedFunction
    direction = '1E-4 0 0'
    expression = 1-((n+1)*(x^n))+(n*(x^(n+1)))
    symbol_names = 'n'
    symbol_values = '5'
  [../]
  [./answer_d2Q2PRelPermPowerGas]
    type = Grad2ParsedFunction
    direction = '1E-5 0 0'
    expression = 1-((n+1)*(x^n))+(n*(x^(n+1)))
    symbol_names = 'n'
    symbol_values = '5'
  [../]
[]

[AuxVariables]
  [./RelPermPower_Aux]
  [../]
  [./dRelPermPower_Aux]
  [../]
  [./d2RelPermPower_Aux]
  [../]

  [./RelPermPower5_Aux]
  [../]
  [./dRelPermPower5_Aux]
  [../]
  [./d2RelPermPower5_Aux]
  [../]

  [./RelPermVG_Aux]
  [../]
  [./dRelPermVG_Aux]
  [../]
  [./d2RelPermVG_Aux]
  [../]

  [./RelPermVG1_Aux]
  [../]
  [./dRelPermVG1_Aux]
  [../]
  [./d2RelPermVG1_Aux]
  [../]

  [./RelPermBW_Aux]
  [../]
  [./dRelPermBW_Aux]
  [../]
  [./d2RelPermBW_Aux]
  [../]

  [./RelPermMonomial_Aux]
  [../]
  [./dRelPermMonomial_Aux]
  [../]
  [./d2RelPermMonomial_Aux]
  [../]

  [./RelPermPowerGas_Aux]
  [../]
  [./dRelPermPowerGas_Aux]
  [../]
  [./d2RelPermPowerGas_Aux]
  [../]

  [./Q2PRelPermPowerGas_Aux]
  [../]
  [./dQ2PRelPermPowerGas_Aux]
  [../]
  [./d2Q2PRelPermPowerGas_Aux]
  [../]

  [./RelPermMonomial_zero_Aux]
  [../]
  [./dRelPermMonomial_zero_Aux]
  [../]
  [./d2RelPermMonomial_zero_Aux]
  [../]

  [./check_Aux]
  [../]
[]

[AuxKernels]
  [./RelPermPower_AuxK]
    type = RichardsRelPermAux
    variable = RelPermPower_Aux
    relperm_UO = RelPermPower
    seff_var = pressure
  [../]
  [./dRelPermPower_AuxK]
    type = RichardsRelPermPrimeAux
    variable = dRelPermPower_Aux
    relperm_UO = RelPermPower
    seff_var = pressure
  [../]
  [./d2RelPermPower_AuxK]
    type = RichardsRelPermPrimePrimeAux
    variable = d2RelPermPower_Aux
    relperm_UO = RelPermPower
    seff_var = pressure
  [../]

  [./RelPermPower5_AuxK]
    type = RichardsRelPermAux
    variable = RelPermPower5_Aux
    relperm_UO = RelPermPower5
    seff_var = pressure
  [../]
  [./dRelPermPower5_AuxK]
    type = RichardsRelPermPrimeAux
    variable = dRelPermPower5_Aux
    relperm_UO = RelPermPower5
    seff_var = pressure
  [../]
  [./d2RelPermPower5_AuxK]
    type = RichardsRelPermPrimePrimeAux
    variable = d2RelPermPower5_Aux
    relperm_UO = RelPermPower5
    seff_var = pressure
  [../]

  [./RelPermVG_AuxK]
    type = RichardsRelPermAux
    variable = RelPermVG_Aux
    relperm_UO = RelPermVG
    seff_var = pressure
  [../]
  [./dRelPermVG_AuxK]
    type = RichardsRelPermPrimeAux
    variable = dRelPermVG_Aux
    relperm_UO = RelPermVG
    seff_var = pressure
  [../]
  [./d2RelPermVG_AuxK]
    type = RichardsRelPermPrimePrimeAux
    variable = d2RelPermVG_Aux
    relperm_UO = RelPermVG
    seff_var = pressure
  [../]

  [./RelPermVG1_AuxK]
    type = RichardsRelPermAux
    variable = RelPermVG1_Aux
    relperm_UO = RelPermVG1
    seff_var = pressure
  [../]
  [./dRelPermVG1_AuxK]
    type = RichardsRelPermPrimeAux
    variable = dRelPermVG1_Aux
    relperm_UO = RelPermVG1
    seff_var = pressure
  [../]
  [./d2RelPermVG1_AuxK]
    type = RichardsRelPermPrimePrimeAux
    variable = d2RelPermVG1_Aux
    relperm_UO = RelPermVG1
    seff_var = pressure
  [../]

  [./RelPermBW_AuxK]
    type = RichardsRelPermAux
    variable = RelPermBW_Aux
    relperm_UO = RelPermBW
    seff_var = pressure
  [../]
  [./dRelPermBW_AuxK]
    type = RichardsRelPermPrimeAux
    variable = dRelPermBW_Aux
    relperm_UO = RelPermBW
    seff_var = pressure
  [../]
  [./d2RelPermBW_AuxK]
    type = RichardsRelPermPrimePrimeAux
    variable = d2RelPermBW_Aux
    relperm_UO = RelPermBW
    seff_var = pressure
  [../]

  [./RelPermMonomial_AuxK]
    type = RichardsRelPermAux
    variable = RelPermMonomial_Aux
    relperm_UO = RelPermMonomial
    seff_var = pressure
  [../]
  [./dRelPermMonomial_AuxK]
    type = RichardsRelPermPrimeAux
    variable = dRelPermMonomial_Aux
    relperm_UO = RelPermMonomial
    seff_var = pressure
  [../]
  [./d2RelPermMonomial_AuxK]
    type = RichardsRelPermPrimePrimeAux
    variable = d2RelPermMonomial_Aux
    relperm_UO = RelPermMonomial
    seff_var = pressure
  [../]

  [./RelPermPowerGas_AuxK]
    type = RichardsRelPermAux
    variable = RelPermPowerGas_Aux
    relperm_UO = RelPermPowerGas
    seff_var = pressure
  [../]
  [./dRelPermPowerGas_AuxK]
    type = RichardsRelPermPrimeAux
    variable = dRelPermPowerGas_Aux
    relperm_UO = RelPermPowerGas
    seff_var = pressure
  [../]
  [./d2RelPermPowerGas_AuxK]
    type = RichardsRelPermPrimePrimeAux
    variable = d2RelPermPowerGas_Aux
    relperm_UO = RelPermPowerGas
    seff_var = pressure
  [../]

  [./Q2PRelPermPowerGas_AuxK]
    type = RichardsRelPermAux
    variable = Q2PRelPermPowerGas_Aux
    relperm_UO = Q2PRelPermPowerGas
    seff_var = pressure
  [../]
  [./dQ2PRelPermPowerGas_AuxK]
    type = RichardsRelPermPrimeAux
    variable = dQ2PRelPermPowerGas_Aux
    relperm_UO = Q2PRelPermPowerGas
    seff_var = pressure
  [../]
  [./d2Q2PRelPermPowerGas_AuxK]
    type = RichardsRelPermPrimePrimeAux
    variable = d2Q2PRelPermPowerGas_Aux
    relperm_UO = Q2PRelPermPowerGas
    seff_var = pressure
  [../]

  [./RelPermMonomial_zero_AuxK]
    type = RichardsRelPermAux
    variable = RelPermMonomial_zero_Aux
    relperm_UO = RelPermMonomial_zero
    seff_var = pressure
  [../]
  [./dRelPermMonomial_zero_AuxK]
    type = RichardsRelPermPrimeAux
    variable = dRelPermMonomial_zero_Aux
    relperm_UO = RelPermMonomial_zero
    seff_var = pressure
  [../]
  [./d2RelPermMonomial_zero_AuxK]
    type = RichardsRelPermPrimePrimeAux
    variable = d2RelPermMonomial_zero_Aux
    relperm_UO = RelPermMonomial_zero
    seff_var = pressure
  [../]

  [./check_AuxK]
    type = FunctionAux
    variable = check_Aux
    function = answer_RelPermBW
  [../]
[]

[Postprocessors]
  [./cf_RelPermPower]
    type = NodalL2Error
    function = answer_RelPermPower
    variable = RelPermPower_Aux
  [../]
  [./cf_dRelPermPower]
    type = NodalL2Error
    function = answer_dRelPermPower
    variable = dRelPermPower_Aux
  [../]
  [./cf_d2RelPermPower]
    type = NodalL2Error
    function = answer_d2RelPermPower
    variable = d2RelPermPower_Aux
  [../]

  [./cf_RelPermPower5]
    type = NodalL2Error
    function = answer_RelPermPower5
    variable = RelPermPower5_Aux
  [../]
  [./cf_dRelPermPower5]
    type = NodalL2Error
    function = answer_dRelPermPower5
    variable = dRelPermPower5_Aux
  [../]
  [./cf_d2RelPermPower5]
    type = NodalL2Error
    function = answer_d2RelPermPower5
    variable = d2RelPermPower5_Aux
  [../]

  [./cf_RelPermVG]
    type = NodalL2Error
    function = answer_RelPermVG
    variable = RelPermVG_Aux
  [../]
  [./cf_dRelPermVG]
    type = NodalL2Error
    function = answer_dRelPermVG
    variable = dRelPermVG_Aux
  [../]
  [./cf_d2RelPermVG]
    type = NodalL2Error
    function = answer_d2RelPermVG
    variable = d2RelPermVG_Aux
  [../]

  [./cf_RelPermVG1]
    type = NodalL2Error
    function = answer_RelPermVG1
    variable = RelPermVG1_Aux
  [../]
  [./cf_dRelPermVG1]
    type = NodalL2Error
    function = answer_dRelPermVG1
    variable = dRelPermVG1_Aux
  [../]
  [./cf_d2RelPermVG1]
    type = NodalL2Error
    function = answer_d2RelPermVG1
    variable = d2RelPermVG1_Aux
  [../]

  [./cf_RelPermBW]
    type = NodalL2Error
    function = answer_RelPermBW
    variable = RelPermBW_Aux
  [../]
  [./cf_dRelPermBW]
    type = NodalL2Error
    function = answer_dRelPermBW
    variable = dRelPermBW_Aux
  [../]
  [./cf_d2RelPermBW]
    type = NodalL2Error
    function = answer_d2RelPermBW
    variable = d2RelPermBW_Aux
  [../]

  [./cf_RelPermMonomial]
    type = NodalL2Error
    function = answer_RelPermMonomial
    variable = RelPermMonomial_Aux
  [../]
  [./cf_dRelPermMonomial]
    type = NodalL2Error
    function = answer_dRelPermMonomial
    variable = dRelPermMonomial_Aux
  [../]
  [./cf_d2RelPermMonomial]
    type = NodalL2Error
    function = answer_d2RelPermMonomial
    variable = d2RelPermMonomial_Aux
  [../]

  [./cf_RelPermPowerGas]
    type = NodalL2Error
    function = answer_RelPermPowerGas
    variable = RelPermPowerGas_Aux
  [../]
  [./cf_dRelPermPowerGas]
    type = NodalL2Error
    function = answer_dRelPermPowerGas
    variable = dRelPermPowerGas_Aux
  [../]
  [./cf_d2RelPermPowerGas]
    type = NodalL2Error
    function = answer_d2RelPermPowerGas
    variable = d2RelPermPowerGas_Aux
  [../]

  [./cf_Q2PRelPermPowerGas]
    type = NodalL2Error
    function = answer_Q2PRelPermPowerGas
    variable = Q2PRelPermPowerGas_Aux
  [../]
  [./cf_dQ2PRelPermPowerGas]
    type = NodalL2Error
    function = answer_dQ2PRelPermPowerGas
    variable = dQ2PRelPermPowerGas_Aux
  [../]
  [./cf_d2Q2PRelPermPowerGas]
    type = NodalL2Error
    function = answer_d2Q2PRelPermPowerGas
    variable = d2Q2PRelPermPowerGas_Aux
  [../]

  [./cf_RelPermMonomial_zero]
    type = NodalL2Error
    function = answer_RelPermMonomial_zero
    variable = RelPermMonomial_zero_Aux
  [../]
  [./cf_dRelPermMonomial_zero]
    type = NodalL2Error
    function = answer_dRelPermMonomial_zero
    variable = dRelPermMonomial_zero_Aux
  [../]
  [./cf_d2RelPermMonomial_zero]
    type = NodalL2Error
    function = answer_d2RelPermMonomial_zero
    variable = d2RelPermMonomial_zero_Aux
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
  xmin = 0.01
  xmax = 0.99
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
    relperm_UO = RelPermPower_unimportant
    sat_UO = Saturation
    seff_UO = SeffVG
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
    petsc_options_value = 'bcgs bjacobi 1E50 .999 10000'
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
  file_base = uo1
  [./csv]
    type = CSV
    [../]
  [./exodus]
    type = Exodus
    hide = pressure
  [../]
[]
