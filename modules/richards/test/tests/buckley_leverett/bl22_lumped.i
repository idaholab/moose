# two-phase version
# super-sharp front version
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 150
  xmin = 0
  xmax = 15
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
    y = '1E-4 1E-3 1E-2 2E-2 5E-2 6E-2 0.1 0.2'
    x = '0    1E-2 1E-1 1    5    20   40  41'
  [../]
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = 'pwater pgas'
  [../]
  [./DensityWater]
    type = RichardsDensityConstBulk
    dens0 = 1000
    bulk_mod = 2E6
  [../]
  [./DensityGas]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 2E6
  [../]
  [./SeffWater]
    type = RichardsSeff2waterVG
    m = 0.8
    al = 1E-4
  [../]
  [./SeffGas]
    type = RichardsSeff2gasVG
    m = 0.8
    al = 1E-4
  [../]
  [./RelPermWater]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./RelPermGas]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./SatWater]
    type = RichardsSat
    s_res = 0.0
    sum_s_res = 0.0
  [../]
  [./SatGas]
    type = RichardsSat
    s_res = 0.0
    sum_s_res = 0.0
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

[AuxVariables]
  [./Seff1VG_Aux]
  [../]
  [./bounds_dummy]
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
  [./richardsppenalty]
    type = RichardsPPenalty
    variable = pgas
    a = 1E-18
    lower_var = pwater
  [../]
[]

[AuxKernels]
  [./Seff1VG_AuxK]
    type = RichardsSeffAux
    variable = Seff1VG_Aux
    seff_UO = SeffWater
    pressure_vars = 'pwater pgas'
  [../]
[]

[ICs]
  [./water_ic]
    type = FunctionIC
    variable = pwater
    function = initial_water
  [../]
  [./gas_ic]
    type = FunctionIC
    variable = pgas
    function = initial_gas
  [../]
[]

[BCs]
  [./left_w]
    type = DirichletBC
    variable = pwater
    boundary = left
    value = 1E6
  [../]
  [./left_g]
    type = DirichletBC
    variable = pgas
    boundary = left
    value = 1000
  [../]
  [./right_w]
    type = DirichletBC
    variable = pwater
    boundary = right
    value = -100000
  [../]
  [./right_g]
    type = DirichletBC
    variable = pgas
    boundary = right
    value = 0
  [../]
[]


[Functions]
  [./initial_water]
    type = ParsedFunction
    expression = 1000000*(1-min(x/5,1))-if(x<5,0,100000)
  [../]
  [./initial_gas]
    type = ParsedFunction
    expression = 1000
  [../]
[]


[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.15
    mat_permeability = '1E-10 0 0  0 1E-10 0  0 0 1E-10'
    viscosity = '1E-3 1E-6'
    gravity = '0 0 0'
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]

  [./standard]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it -ksp_rtol -ksp_atol'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-10 1E-10 20 1E-10 1E-100'
  [../]

[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 50

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = bl22_lumped
  [./exodus]
    type = Exodus
    interval = 100000
    hide = 'pgas bounds_dummy'
    execute_on = 'initial final timestep_end'
  [../]
[]
