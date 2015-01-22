# checks the rotation matrices
# no other computations are relevant
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -10
  xmax = 10
  ymin = -10
  ymax = 10
  zmin = -10
  zmax = 10
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
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
    point_file = bh01.bh
    SumQuantityUO = borehole_total_outflow_mass
    variable = pressure
    unit_weight = '0 0 0'
    character = zero
    MyNameIsAndyWilkins = true
  [../]
[]


[Postprocessors]
  [./bh_report]
    type = RichardsPlotQuantity
    uo = borehole_total_outflow_mass
  [../]
[]


[Functions]
  [./initial_pressure]
    type = ParsedFunction
    value = 0
  [../]
  [./zero]
    type = ConstantFunction
    value = 0
  [../]
[]


[Materials]
  [./all]
    type = RichardsMaterial
    block = 0
    viscosity = 0.3
    mat_porosity = 0.1
    mat_permeability = '1 2 3 2 2.22 4 3 4 4.44'
    density_UO = DensityConstBulk
    relperm_UO = RelPermPower
    sat_UO = Saturation
    seff_UO = Seff1VG
    SUPG_UO = SUPGstandard
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
    petsc_options_value = 'bcgs bjacobi 1 1 10000 30'
  [../]
[]


[Executioner]
  type = Transient   # Here we use the Transient Executioner
  end_time = 1
  num_steps = 1
  solve_type = NEWTON

[]

[Outputs]
  file_base = bh01
  output_initial = true
  exodus = false
  csv = true
  print_linear_residuals = true
  print_perf_log = true
[]
