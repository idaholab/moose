# with fully_upwind sink
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
[]


[GlobalParams]
  richardsVarNames_UO = PPNames
  density_UO = DensityConstBulk
  relperm_UO = RelPermPower
  SUPG_UO = SUPGstandard
  sat_UO = Saturation
  seff_UO = SeffVG
  fully_upwind = true
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
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = initial_pressure
    [../]
  [../]
[]

[Functions]
  [./initial_pressure]
    type = ParsedFunction
    expression = 2
  [../]

  [./mass_bal_fcn]
    type = ParsedFunction
    expression = abs((mi-lfout-rfout-mf)/2/(mi+mf))
    symbol_names = 'mi mf lfout rfout'
    symbol_values = 'mass_init mass_fin left_flux_out right_flux_out'
  [../]
[]

[Postprocessors]
  [./mass_init]
    type = RichardsMass
    variable = pressure
    execute_on = timestep_begin
  [../]
  [./mass_fin]
    type = RichardsMass
    variable = pressure
    execute_on = timestep_end
  [../]
  [./left_flux_out]
    type = RichardsPiecewiseLinearSinkFlux
    boundary = left
    variable = pressure
    pressures = '0 1'
    bare_fluxes = '1 2'
    use_mobility = false
    use_relperm = false
  [../]
  [./right_flux_out]
    type = RichardsPiecewiseLinearSinkFlux
    boundary = right
    variable = pressure
    pressures = '0 1'
    bare_fluxes = '1 2'
    use_mobility = false
    use_relperm = false
  [../]
  [./p0]
    type = PointValue
    point = '0 0 0'
    variable = pressure
  [../]
  [./mass_bal]
    type = FunctionValuePostprocessor
    function = mass_bal_fcn
  [../]
[]

[BCs]
  [./left_flux]
    type = RichardsPiecewiseLinearSink
    boundary = left
    pressures = '0 1'
    bare_fluxes = '1 2'
    variable = pressure
    use_mobility = false
    use_relperm = false
  [../]
  [./right_flux]
    type = RichardsPiecewiseLinearSink
    boundary = right
    pressures = '0 1'
    bare_fluxes = '1 2'
    variable = pressure
    use_mobility = false
    use_relperm = false
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
    viscosity = 1E-3
    gravity = '-1 0 0'
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]
  active = 'andy'
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
  dt = 2E-3
  end_time = 0.2
  nl_abs_tol = 1E-12
  nl_rel_tol = 1E-10
[]

[Outputs]
  file_base = s_fu_01
  csv = true
  execute_on = timestep_end
[]
