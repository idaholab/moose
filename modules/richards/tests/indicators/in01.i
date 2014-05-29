[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  xmin = -1
  xmax = 1
  zmin = -1
  zmax = 1
  ymin = -1
  ymax = 1
[]


[GlobalParams]
  richardsVarNames_UO = PPNames
  density_UO = DensityConstBulk
  relperm_UO = RelPermPower
  SUPG_UO = SUPGstandard
  sat_UO = Saturation
  seff_UO = SeffVG
  viscosity = 1E-3
  gravity = '0 0 0'
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = pressure
  [../]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 1.0E3
  [../]
  [./SeffVG]
    type = RichardsSeff1VG
    m = 0.8
    al = 1
  [../]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.0
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
[]

[Variables]
  [./pressure]
  [../]
[]

[ICs]
  [./p_init]
    type = FunctionIC
    function = 2*x+(2*y)+(2*z)
    variable = pressure
  [../]
[]


[Kernels]
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
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    #petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  [../]
[]

[Adaptivity]
  marker = errorfrac
  max_h_level = 1
  [./Indicators]
    [./mobjump]
      type = RichardsMobilityJumpIndicator
      variable = pressure
    [../]
    [./fluxjump]
      type = FluxJumpIndicator
      property = porosity
      variable = pressure
    [../]
  [../]
  [./Markers]
    [./errorfrac]
      type = ErrorFractionMarker
      refine = 0.4
      coarsen = 0.1
      indicator = mobjump
    [../]
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 3
[]

[Outputs]
  file_base = in01
  output_initial = true
  output_final = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
