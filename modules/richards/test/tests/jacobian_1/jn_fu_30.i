# unsaturated = true
# gravity = true
# supg = true
# transient = true
# wellbore = true

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
  density_UO = DensityConstBulk
  relperm_UO = RelPermPower
  SUPG_UO = SUPGstandard
  sat_UO = Saturation
  seff_UO = SeffVG
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = pressure
  [../]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 1.0E0 # notice small quantity, so the PETSc constant state works
  [../]
  [./SeffVG]
    type = RichardsSeff1VG
    m = 0.8
    al = 1 # notice small quantity, so the PETSc constant state works
  [../]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.2
    n = 2
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0.1
    sum_s_res = 0.1
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 0.1
  [../]

  [./borehole_total_outflow_mass]
    type = RichardsSumQuantity
  [../]
[]

[Variables]
  [./pressure]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = RandomIC
      block = 0
      min = 0
      max = 1
    [../]
  [../]
[]

[DiracKernels]
  [./bh]
    type = RichardsBorehole
    bottom_pressure = 0
    point_file = jn30.bh
    SumQuantityUO = borehole_total_outflow_mass
    variable = pressure
    unit_weight = '0 0 0'
    character = 1E12
    fully_upwind = true
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
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
    viscosity = 1E-3
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
  dt = 1E-5
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = jn_fu_30
  exodus = false
[]
