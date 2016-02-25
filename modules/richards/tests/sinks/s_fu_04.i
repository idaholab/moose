# apply a total flux (in kg/s) to two boundaries
# and check that it removes the correct amount of fluid
# fully-upwind sink
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 4
[]


[GlobalParams]
  richardsVarNames_UO = PPNames
  density_UO = DensityConstBulk
  relperm_UO = RelPermPower
  SUPG_UO = SUPGstandard
  sat_UO = Saturation
  seff_UO = SeffVG
  viscosity = 1E-3
  gravity = '-1 0 0'
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = pressure
  [../]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 1
  [../]
  [./SeffVG]
    type = RichardsSeff1VG
    m = 0.5
    al = 1 # same deal with PETSc constant state
  [../]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0.1
    sum_s_res = 0.2
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 0.1
  [../]
[]

[Variables]
  [./pressure]
  [../]
[]

[ICs]
  [./pressure]
    type = ConstantIC
    variable = pressure
    value = 2
  [../]
[]

[Postprocessors]
  [./area_left]
    type = AreaPostprocessor
    boundary = left
    execute_on = initial
  [../]
  [./area_right]
    type = AreaPostprocessor
    boundary = right
    execute_on = initial
  [../]
  [./mass_fin]
    type = RichardsMass
    variable = pressure
    execute_on = 'initial timestep_end'
  [../]
  [./p0]
    type = PointValue
    point = '0 0 0'
    variable = pressure
    execute_on = 'initial timestep_end'
  [../]
[]

[BCs]
  [./left_flux]
    type = RichardsPiecewiseLinearSink
    boundary = left
    pressures = '0'
    bare_fluxes = '0.1'
    variable = pressure
    use_mobility = false
    use_relperm = false
    area_pp = area_left
    fully_upwind = true
  [../]
  [./right_flux]
    type = RichardsPiecewiseLinearSink
    boundary = right
    pressures = '0'
    bare_fluxes = '0.1'
    variable = pressure
    use_mobility = false
    use_relperm = false
    area_pp = area_right
    fully_upwind = true
  [../]
[]

[Kernels]
  active = 'richardst'
  [./richardst]
    type = RichardsMassChange
    variable = pressure
  [../]
[]

[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-10 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 13
[]

[Outputs]
  file_base = s_fu_04
  csv = true
[]
