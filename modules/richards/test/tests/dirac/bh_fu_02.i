# fully-saturated
# production
# fullyupwind
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
  sat_UO = Saturation
  seff_UO = Seff1VG
  SUPG_UO = SUPGstandard
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = pressure
  [../]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1000
    bulk_mod = 2E9
  [../]
  [./Seff1VG]
    type = RichardsSeff1VG
    m = 0.8
    al = 1E-5
  [../]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0
    sum_s_res = 0
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 1E8
  [../]

  [./borehole_total_outflow_mass]
    type = RichardsSumQuantity
  [../]
[]


[Variables]
  active = 'pressure'
  [./pressure]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./p_ic]
    type = FunctionIC
    variable = pressure
    function = initial_pressure
  [../]
[]

[AuxVariables]
  [./Seff1VG_Aux]
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

[DiracKernels]
  [./bh]
    type = RichardsBorehole
    bottom_pressure = 0
    point_file = bh02.bh
    SumQuantityUO = borehole_total_outflow_mass
    variable = pressure
    unit_weight = '0 0 0'
    character = 1
    fully_upwind = true
  [../]
[]


[Postprocessors]
  [./bh_report]
    type = RichardsPlotQuantity
    uo = borehole_total_outflow_mass
  [../]

  [./fluid_mass0]
    type = RichardsMass
    variable = pressure
    execute_on = timestep_begin
  [../]

  [./fluid_mass1]
    type = RichardsMass
    variable = pressure
    execute_on = timestep_end
  [../]

  [./zmass_error]
    type = FunctionValuePostprocessor
    function = mass_bal_fcn
    execute_on = timestep_end
    indirect_dependencies = 'fluid_mass1 fluid_mass0 bh_report'
  [../]

  [./p0]
    type = PointValue
    variable = pressure
    point = '1 1 1'
    execute_on = timestep_end
  [../]
[]


[Functions]
  [./initial_pressure]
    type = ParsedFunction
    expression = 1E7
  [../]

  [./mass_bal_fcn]
    type = ParsedFunction
    expression = abs((a-c+d)/2/(a+c))
    symbol_names = 'a c d'
    symbol_values = 'fluid_mass1 fluid_mass0 bh_report'
  [../]
[]


[Materials]
  [./all]
    type = RichardsMaterial
    block = 0
    viscosity = 1E-3
    mat_porosity = 0.1
    mat_permeability = '1E-12 0 0  0 1E-12 0  0 0 1E-12'
    gravity = '0 0 0'
    linear_shape_fcns = true
  [../]
[]

[AuxKernels]
  [./Seff1VG_AuxK]
    type = RichardsSeffAux
    variable = Seff1VG_Aux
    seff_UO = Seff1VG
    pressure_vars = pressure
  [../]
[]


[Preconditioning]
  [./usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10000 30'
  [../]
[]


[Executioner]
  type = Transient
  end_time = 0.5
  dt = 1E-2
  solve_type = NEWTON

[]

[Outputs]
  file_base = bh_fu_02
  exodus = false
  csv = true
  execute_on = timestep_end
[]
