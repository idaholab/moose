# Density User objects give the correct value
#
# If you want to add another test for another UserObject
# then add the UserObject, add a Function defining the expected result,
# add an AuxVariable and AuxKernel that will record the UserObjects value
# and finally add a NodalL2Error that compares this with the Function

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = pressure
  [../]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1000
    bulk_mod = 2E6
  [../]
  [./DensityIdeal]
    type = RichardsDensityIdeal
    p0 = 33333
    slope = 1.1E-2
  [../]
  [./DensityMethane20degC]
    type = RichardsDensityMethane20degC
  [../]
  [./DensityVDW]
    type = RichardsDensityVDW
    a = 0.2303
    b = 4.31E-4
    temperature = 293
    molar_mass = 16.04246E-3
    infinity_ratio = 10
  [../]
  [./DensityConstBulkCut]
    type = RichardsDensityConstBulkCut
    dens0 = 1000
    bulk_mod = 2E6
    cut_limit = 1E6
    zero_point = -1E6
  [../]

  # following are unimportant in this test
  [./SeffVG]
    type = RichardsSeff1VG
    m = 0.8
    al = 1E-6
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
  [./answer_DensityConstBulk]
    type = ParsedFunction
    expression = dens0*exp(x/bulk_mod)
    symbol_names = 'dens0 bulk_mod'
    symbol_values = '1000 2E6'
  [../]
  [./answer_dDensityConstBulk]
    type = GradParsedFunction
    direction = '1 0 0'
    expression = dens0*exp(x/bulk_mod)
    symbol_names = 'dens0 bulk_mod'
    symbol_values = '1000 2E6'
  [../]
  [./answer_d2DensityConstBulk]
    type = Grad2ParsedFunction
    direction = '1 0 0'
    expression = dens0*exp(x/bulk_mod)
    symbol_names = 'dens0 bulk_mod'
    symbol_values = '1000 2E6'
  [../]

  [./answer_DensityIdeal]
    type = ParsedFunction
    expression = slope*(x-p0)
    symbol_names = 'p0 slope'
    symbol_values = '33333 1.1E-2'
  [../]
  [./answer_dDensityIdeal]
    type = GradParsedFunction
    direction = '1 0 0'
    expression = slope*(x-p0)
    symbol_names = 'p0 slope'
    symbol_values = '33333 1.1E-2'
  [../]
  [./answer_d2DensityIdeal]
    type = Grad2ParsedFunction
    direction = '1 0 0'
    expression = slope*(x-p0)
    symbol_names = 'p0 slope'
    symbol_values = '33333 1.1E-2'
  [../]

  [./answer_DensityMethane20degC]
    type = ParsedFunction
    expression = if(x>0,(0.00654576947608E-3*x+1.04357716547E-13*x^2),0)+if(x<0,0.1*(e^(6.54576947608E-5*x)-1),0)
  [../]
  [./answer_dDensityMethane20degC]
    type = GradParsedFunction
    direction = '1 0 0'
    expression = if(x>0,(0.00654576947608E-3*x+1.04357716547E-13*x^2),0)+if(x<0,0.1*(e^(6.54576947608E-5*x)-1),0)
  [../]
  [./answer_d2DensityMethane20degC]
    type = Grad2ParsedFunction
    direction = '1 0 0'
    expression = if(x>0,(0.00654576947608E-3*x+1.04357716547E-13*x^2),0)+if(x<0,0.1*(e^(6.54576947608E-5*x)-1),0)
  [../]

  [./answer_DensityVDW]
    type = ParsedFunction
    expression = if(x>0,-(molar_mass*(-2+(2*pow(2,0.3333333333333333)*(a-3*b*(b*x+rt)))/pow(-2*pow(a,3)+9*pow(a,2)*b*(-2*b*x+rt)+pow(pow(a,3)*(a*pow(2*a+9*b*(2*b*x-rt),2)-4*pow(a-3*b*(b*x+rt),3)),0.5),0.3333333333333333)+(pow(2,0.6666666666666666)*pow(-2*pow(a,3)+9*pow(a,2)*b*(-2*b*x+rt)+pow(pow(a,3)*(a*pow(2*a+9*b*(2*b*x-rt),2)-4*pow(a-3*b*(b*x+rt),3)),0.5),0.3333333333333333))/a))/(6.*b)+(molar_mass*(-2+(2*pow(2,0.3333333333333333)*(a-3*b*(b*0+rt)))/pow(-2*pow(a,3)+9*pow(a,2)*b*(-2*b*0+rt)+pow(pow(a,3)*(a*pow(2*a+9*b*(2*b*0-rt),2)-4*pow(a-3*b*(b*0+rt),3)),0.5),0.3333333333333333)+(pow(2,0.6666666666666666)*pow(-2*pow(a,3)+9*pow(a,2)*b*(-2*b*0+rt)+pow(pow(a,3)*(a*pow(2*a+9*b*(2*b*0-rt),2)-4*pow(a-3*b*(b*0+rt),3)),0.5),0.3333333333333333))/a))/(6.*b),infinityratio*molar_mass*(e^(slope0*x)-1))
    symbol_names = 'a b rt molar_mass infinityratio slope0'
    symbol_values = '0.2303 0.000431 2436.1403 0.01604246 10 4.10485e-05'
  [../]
  [./answer_dDensityVDW]
    type = GradParsedFunction
    direction = '1 0 0'
    expression = if(x>0,-(molar_mass*(-2+(2*pow(2,0.3333333333333333)*(a-3*b*(b*x+rt)))/pow(-2*pow(a,3)+9*pow(a,2)*b*(-2*b*x+rt)+pow(pow(a,3)*(a*pow(2*a+9*b*(2*b*x-rt),2)-4*pow(a-3*b*(b*x+rt),3)),0.5),0.3333333333333333)+(pow(2,0.6666666666666666)*pow(-2*pow(a,3)+9*pow(a,2)*b*(-2*b*x+rt)+pow(pow(a,3)*(a*pow(2*a+9*b*(2*b*x-rt),2)-4*pow(a-3*b*(b*x+rt),3)),0.5),0.3333333333333333))/a))/(6.*b),infinityratio*molar_mass*(e^(slope0*x)-1))
    symbol_names = 'a b rt molar_mass infinityratio slope0'
    symbol_values = '0.2303 0.000431 2436.1403 0.01604246 10 4.10485e-05'
  [../]
  [./answer_d2DensityVDW]
    type = Grad2ParsedFunction
    direction = '1 0 0'
    expression = if(x>0,-(molar_mass*(-2+(2*pow(2,0.3333333333333333)*(a-3*b*(b*x+rt)))/pow(-2*pow(a,3)+9*pow(a,2)*b*(-2*b*x+rt)+pow(pow(a,3)*(a*pow(2*a+9*b*(2*b*x-rt),2)-4*pow(a-3*b*(b*x+rt),3)),0.5),0.3333333333333333)+(pow(2,0.6666666666666666)*pow(-2*pow(a,3)+9*pow(a,2)*b*(-2*b*x+rt)+pow(pow(a,3)*(a*pow(2*a+9*b*(2*b*x-rt),2)-4*pow(a-3*b*(b*x+rt),3)),0.5),0.3333333333333333))/a))/(6.*b),infinityratio*molar_mass*(e^(slope0*x)-1))
    symbol_names = 'a b rt molar_mass infinityratio slope0'
    symbol_values = '0.2303 0.000431 2436.1403 0.01604246 10 4.10485e-05'
  [../]

  [./answer_DensityConstBulkCut]
    type = ParsedFunction
    expression = if(x<zero_pt,0,if(x>cut_limit,dens0*exp(x/bulk_mod),(3*cut_limit-2*x-zero_pt)*(x-zero_pt)*(x-zero_pt)*dens0*exp(x/bulk_mod)/(cut_limit-zero_pt)/(cut_limit-zero_pt)/(cut_limit-zero_pt)))
    symbol_names = 'dens0 bulk_mod zero_pt cut_limit'
    symbol_values = '1000 2E6 -1E6 1E6'
  [../]
  [./answer_dDensityConstBulkCut]
    type = GradParsedFunction
    direction = '1 0 0'
    expression = if(x<zero_pt,0,if(x>cut_limit,dens0*exp(x/bulk_mod),(3*cut_limit-2*x-zero_pt)*(x-zero_pt)*(x-zero_pt)*dens0*exp(x/bulk_mod)/(cut_limit-zero_pt)/(cut_limit-zero_pt)/(cut_limit-zero_pt)))
    symbol_names = 'dens0 bulk_mod zero_pt cut_limit'
    symbol_values = '1000 2E6 -1E6 1E6'
  [../]
  [./answer_d2DensityConstBulkCut]
    type = Grad2ParsedFunction
    direction = '1 0 0'
    expression = if(x<zero_pt,0,if(x>cut_limit,dens0*exp(x/bulk_mod),(3*cut_limit-2*x-zero_pt)*(x-zero_pt)*(x-zero_pt)*dens0*exp(x/bulk_mod)/(cut_limit-zero_pt)/(cut_limit-zero_pt)/(cut_limit-zero_pt)))
    symbol_names = 'dens0 bulk_mod zero_pt cut_limit'
    symbol_values = '1000 2E6 -1E6 1E6'
  [../]
[]

[AuxVariables]
  [./DensityConstBulk_Aux]
  [../]
  [./dDensityConstBulk_Aux]
  [../]
  [./d2DensityConstBulk_Aux]
  [../]

  [./DensityIdeal_Aux]
  [../]
  [./dDensityIdeal_Aux]
  [../]
  [./d2DensityIdeal_Aux]
  [../]

  [./DensityMethane20degC_Aux]
  [../]
  [./dDensityMethane20degC_Aux]
  [../]
  [./d2DensityMethane20degC_Aux]
  [../]

  [./DensityVDW_Aux]
  [../]
  [./dDensityVDW_Aux]
  [../]
  [./d2DensityVDW_Aux]
  [../]

  [./DensityConstBulkCut_Aux]
  [../]
  [./dDensityConstBulkCut_Aux]
  [../]
  [./d2DensityConstBulkCut_Aux]
  [../]

  [./check_Aux]
  [../]
[]

[AuxKernels]
  [./DensityConstBulk_AuxK]
    type = RichardsDensityAux
    variable = DensityConstBulk_Aux
    density_UO = DensityConstBulk
    pressure_var = pressure
  [../]
  [./dDensityConstBulk_AuxK]
    type = RichardsDensityPrimeAux
    variable = dDensityConstBulk_Aux
    density_UO = DensityConstBulk
    pressure_var = pressure
  [../]
  [./d2DensityConstBulk_AuxK]
    type = RichardsDensityPrimePrimeAux
    variable = d2DensityConstBulk_Aux
    density_UO = DensityConstBulk
    pressure_var = pressure
  [../]

  [./DensityIdeal_AuxK]
    type = RichardsDensityAux
    variable = DensityIdeal_Aux
    density_UO = DensityIdeal
    pressure_var = pressure
  [../]
  [./dDensityIdeal_AuxK]
    type = RichardsDensityPrimeAux
    variable = dDensityIdeal_Aux
    density_UO = DensityIdeal
    pressure_var = pressure
  [../]
  [./d2DensityIdeal_AuxK]
    type = RichardsDensityPrimePrimeAux
    variable = d2DensityIdeal_Aux
    density_UO = DensityIdeal
    pressure_var = pressure
  [../]

  [./DensityMethane20degC_AuxK]
    type = RichardsDensityAux
    variable = DensityMethane20degC_Aux
    density_UO = DensityMethane20degC
    pressure_var = pressure
  [../]
  [./dDensityMethane20degC_AuxK]
    type = RichardsDensityPrimeAux
    variable = dDensityMethane20degC_Aux
    density_UO = DensityMethane20degC
    pressure_var = pressure
  [../]
  [./d2DensityMethane20degC_AuxK]
    type = RichardsDensityPrimePrimeAux
    variable = d2DensityMethane20degC_Aux
    density_UO = DensityMethane20degC
    pressure_var = pressure
  [../]

  [./DensityVDW_AuxK]
    type = RichardsDensityAux
    variable = DensityVDW_Aux
    density_UO = DensityVDW
    pressure_var = pressure
  [../]
  [./dDensityVDW_AuxK]
    type = RichardsDensityPrimeAux
    variable = dDensityVDW_Aux
    density_UO = DensityVDW
    pressure_var = pressure
  [../]
  [./d2DensityVDW_AuxK]
    type = RichardsDensityPrimePrimeAux
    variable = d2DensityVDW_Aux
    density_UO = DensityVDW
    pressure_var = pressure
  [../]

  [./DensityConstBulkCut_AuxK]
    type = RichardsDensityAux
    variable = DensityConstBulkCut_Aux
    density_UO = DensityConstBulkCut
    pressure_var = pressure
  [../]
  [./dDensityConstBulkCut_AuxK]
    type = RichardsDensityPrimeAux
    variable = dDensityConstBulkCut_Aux
    density_UO = DensityConstBulkCut
    pressure_var = pressure
  [../]
  [./d2DensityConstBulkCut_AuxK]
    type = RichardsDensityPrimePrimeAux
    variable = d2DensityConstBulkCut_Aux
    density_UO = DensityConstBulkCut
    pressure_var = pressure
  [../]

  [./check_AuxK]
    type = FunctionAux
    variable = check_Aux
    function = answer_d2DensityConstBulkCut
  [../]
[]

[Postprocessors]
  [./cf_DensityConstBulk]
    type = NodalL2Error
    function = answer_DensityConstBulk
    variable = DensityConstBulk_Aux
  [../]
  [./cf_dDensityConstBulk]
    type = NodalL2Error
    function = answer_dDensityConstBulk
    variable = dDensityConstBulk_Aux
  [../]
  [./cf_d2DensityConstBulk]
    type = NodalL2Error
    function = answer_d2DensityConstBulk
    variable = d2DensityConstBulk_Aux
  [../]

  [./cf_DensityIdeal]
    type = NodalL2Error
    function = answer_DensityIdeal
    variable = DensityIdeal_Aux
  [../]
  [./cf_dDensityIdeal]
    type = NodalL2Error
    function = answer_dDensityIdeal
    variable = dDensityIdeal_Aux
  [../]
  [./cf_d2DensityIdeal]
    type = NodalL2Error
    function = answer_d2DensityIdeal
    variable = d2DensityIdeal_Aux
  [../]

  [./cf_DensityMethane20degC]
    type = NodalL2Error
    function = answer_DensityMethane20degC
    variable = DensityMethane20degC_Aux
  [../]
  [./cf_dDensityMethane20degC]
    type = NodalL2Error
    function = answer_dDensityMethane20degC
    variable = dDensityMethane20degC_Aux
  [../]
  [./cf_d2DensityMethane20degC]
    type = NodalL2Error
    function = answer_d2DensityMethane20degC
    variable = d2DensityMethane20degC_Aux
  [../]

  [./cf_DensityVDW]
    type = NodalL2Error
    function = answer_DensityVDW
    variable = DensityVDW_Aux
  [../]
  [./cf_dDensityVDW]
    type = NodalL2Error
    function = answer_dDensityVDW
    variable = dDensityVDW_Aux
  [../]
  [./cf_d2DensityVDW]
    type = NodalL2Error
    function = answer_d2DensityVDW
    variable = d2DensityVDW_Aux
  [../]

  [./cf_DensityConstBulkCut]
    type = NodalL2Error
    function = answer_DensityConstBulkCut
    variable = DensityConstBulkCut_Aux
  [../]
  [./cf_dDensityConstBulkCut]
    type = NodalL2Error
    function = answer_dDensityConstBulkCut
    variable = dDensityConstBulkCut_Aux
  [../]
  [./cf_d2DensityConstBulkCut]
    type = NodalL2Error
    function = answer_d2DensityConstBulkCut
    variable = d2DensityConstBulkCut_Aux
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
  active = 'richardst'
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
  file_base = uo2
  [./csv]
    type = CSV
    [../]
  [./exodus]
    type = Exodus
    hide = pressure
  [../]
[]
