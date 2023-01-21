# two phase
# water saturated

# gravity = true
# supg = true
# transient = true


[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
  density_UO = 'DensityWater DensityGas'
  relperm_UO = 'RelPermWater RelPermGas'
  SUPG_UO = 'SUPGwater SUPGgas'
  sat_UO = 'SatWater SatGas'
  seff_UO = 'SeffWater SeffGas'
  viscosity = '1E-3 0.5E-3'
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = 'pwater pgas'
  [../]
  [./DensityWater]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 1.0 # notice small quantity, so the PETSc constant state works
  [../]
  [./DensityGas]
    type = RichardsDensityConstBulk
    dens0 = 0.5
    bulk_mod = 0.5 # notice small quantity, so the PETSc constant state works
  [../]
  [./SeffWater]
    type = RichardsSeff2waterVG
    m = 0.8
    al = 1 # notice small quantity, so the PETSc constant state works
  [../]
  [./SeffGas]
    type = RichardsSeff2gasVG
    m = 0.8
    al = 1 # notice small quantity, so the PETSc constant state works
  [../]
  [./RelPermWater]
    type = RichardsRelPermPower
    simm = 0.2
    n = 2
  [../]
  [./RelPermGas]
    type = RichardsRelPermPower
    simm = 0.1
    n = 3
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
    [./InitialCondition]
      type = FunctionIC
      block = 0
      function = init_p
    [../]
  [../]
  [./pgas]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      block = 0
      function = init_p
    [../]
  [../]
[]

[Functions]
  [./init_p]
    type = ParsedFunction
    expression = x+0.6*y+0.3*z
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

[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
    gravity = '1 2 3'
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E-10
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = jn17
  exodus = false
[]
