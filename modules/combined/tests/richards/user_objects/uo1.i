# User objects give the correct value
# 
# If you want to add another test for another UserObject
# then add the UserObject, add a Function defining the expected result,
# add an AuxVariable and AuxKernel that will record the UserObject's value
# and finally add a NodalL2Error that compares this with the Function

[UserObjects]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1000
    bulk_mod = 2E6
  [../]
  [./DensityIdeal]
    type = RichardsDensityIdeal
    p0 = 33333
    slope = 1.1
  [../]
  [./SeffVG]
    type = RichardsSeffVG
    m = 0.8
    al = 1E-6
  [../]
  [./SeffVG1]
    type = RichardsSeffVG1
    m = 0.8
    al = 1E-6
    p_cut = 1111111
  [../]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.10101
    n = 2
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0.054321
    s_res_air = 0.0
  [../]
[]

[Functions]
  [./initial_pressure]
    type = ParsedFunction
    value = 1E5*(x-50)
  [../]
  [./answer_DensityConstBulk]
    type = ParsedFunction
    value = dens0*exp(1E5*(x-50)/bulk_mod)
    vars = 'dens0 bulk_mod'
    vals = '1000 2E6'
  [../]
  [./answer_DensityIdeal]
    type = ParsedFunction
    value = slope*(1E5*(x-50)-p0)
    vars = 'p0 slope'
    vals = '33333 1.1'
  [../]
  [./answer_SeffVG]
    type = ParsedFunction
    value = (1+max((-1E5*(x-50))*al,0)^(1/(1-m)))^(-m)
    vars = 'al m'
    vals = '1E-6 0.8'
  [../]
  [./answer_SeffVG1] # fix this!
    type = ParsedFunction
    value = min(-1E5*(x-50)-pcut,0)/(-1E5*(x-50)-pcut)*((1+max((-1E5*(x-50))*al,0)^(1/(1-m)))^(-m))
    vars = 'al m pcut'
    vals = '1E-6 0.8 1111111'
  [../]
  [./answer_RelPermPower]
    type = ParsedFunction
    value = max((1+max((-1E5*(x-50))*al,0)^(1/(1-m)))^(-m)-simm,0)/((1+max((-1E5*(x-50))*al,0)^(1/(1-m)))^(-m)-simm)*(((n+1)*(((1+max((-1E5*(x-50))*al,0)^(1/(1-m)))^(-m)-simm)/(1-simm))^n)-(n*((((1+max((-1E5*(x-50))*al,0)^(1/(1-m)))^(-m)-simm)/(1-simm))^(n+1))))
    vars = 'al m simm n'
    vals = '1E-6 0.8 0.10101 2'
  [../]
[]

[AuxVariables]
  [./DensityConstBulk_Aux]
  [../]
  [./DensityIdeal_Aux]
  [../]
  [./SeffVG_Aux]
  [../]
  [./SeffVG1_Aux]
  [../]
  [./RelPermPower_Aux]
  [../]
[]

[AuxKernels]
  [./DensityConstBulk_AuxK]
    type = RichardsDensityAux
    variable = DensityConstBulk_Aux
    density_UO = DensityConstBulk
    pressure_var = pressure
  [../]
  [./DensityIdeal_AuxK]
    type = RichardsDensityAux
    variable = DensityIdeal_Aux
    density_UO = DensityIdeal
    pressure_var = pressure
  [../]
  [./SeffVG_AuxK]
    type = RichardsSeffAux
    variable = SeffVG_Aux
    seff_UO = SeffVG
    p_air = 0.0
    pressure_var = pressure
  [../]
  [./SeffVG1_AuxK]
    type = RichardsSeffAux
    variable = SeffVG1_Aux
    seff_UO = SeffVG1
    p_air = 0.0
    pressure_var = pressure
  [../]
  [./RelPermPower_AuxK]
    type = RichardsRelPermAux
    variable = RelPermPower_Aux
    relperm_UO = RelPermPower
    seff_var = SeffVG_Aux
  [../]
[]

[Postprocessors]
  [./cf_DensityConstBulk]
    type = NodalL2Error
    function = answer_DensityConstBulk
    variable = DensityConstBulk_Aux
  [../]
  [./cf_DensityIdeal]
    type = NodalL2Error
    function = answer_DensityIdeal
    variable = DensityIdeal_Aux
  [../]
  [./cf_SeffVG]
    type = NodalL2Error
    function = answer_SeffVG
    variable = SeffVG_Aux
  [../]
#  [./cf_SeffVG1]
#    type = NodalL2Error
#    function = answer_SeffVG1
#    variable = SeffVG1_Aux
#  [../]
  [./cf_RelPermPower]
    type = NodalL2Error
    function = answer_RelPermPower
    variable = RelPermPower_Aux
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
  xmin = 0
  xmax = 100
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
    variable = pressure
  [../]
  [./richardsf]
    type = RichardsFlux
    variable = pressure
  [../]
[]

[Materials]
  [./unimportant_material]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-20 0 0  0 1E-20 0  0 0 1E-20'
    pressure_variable = pressure
    p_air = 0
    density_UO = DensityConstBulk
    relperm_UO = RelPermPower
    sat_UO = Saturation
    seff_UO = SeffVG
    dens0 = 1000
    viscosity = 1E-3
    gravity = '0 0 -10'
    SUPG = true
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

[Output]
  linear_residuals = false
  file_base = uo1
  interval = 1
  exodus = true
  perf_log = false
  hidden_variables = pressure
[]
