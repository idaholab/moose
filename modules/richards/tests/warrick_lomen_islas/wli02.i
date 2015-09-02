[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 1
  xmin = -1000
  xmax = 0
  ymin = 0
  ymax = 0.05
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
    dens0 = 10
    bulk_mod = 2E9
  [../]
  [./SeffBW]
    type = RichardsSeff1BWsmall
    Sn = 0.0
    Ss = 1.0
    C = 1.5
    las = 2
  [../]
  [./RelPermBW]
    type = RichardsRelPermBW
    Sn = 0.0
    Ss = 1.0
    Kn = 0
    Ks = 1
    C = 1.5
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0.0
    sum_s_res = 0.0
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 1.0E2
  [../]
[]


[Variables]
  active = 'pressure'
  [./pressure]
    order = FIRST
    family = LAGRANGE
    initial_condition = -1E-4
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


[AuxVariables]
  [./Seff1VG_Aux]
  [../]
[]


[AuxKernels]
  [./Seff1VG_AuxK]
    type = RichardsSeffAux
    variable = Seff1VG_Aux
    seff_UO = SeffBW
    pressure_vars = pressure
  [../]
[]


[BCs]
  active = 'base'
  [./base]
    type = DirichletBC
    variable = pressure
    boundary = 'left'
    value = -1E-4
  [../]
[]


[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.25
    mat_permeability = '1 0 0  0 1 0  0 0 1'
    density_UO = DensityConstBulk
    relperm_UO = RelPermBW
    SUPG_UO = SUPGstandard
    sat_UO = Saturation
    seff_UO = SeffBW
    viscosity = 4
    gravity = '-0.1 0 0'
    linear_shape_fcns = true
  [../]
[]

[Preconditioning]
  active = 'andy'

  [./andy]
    type = SMP
    full = true
    petsc_options = ''
    petsc_options_iname = '-ksp_type -pc_type -ksp_rtol -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 1E-10 10000'
  [../]
[]


[Executioner]
  type = Transient
  solve_type = Newton
  petsc_options = '-snes_converged_reason'
  end_time = 100
  dt = 5
[]


[Outputs]
  file_base = wli02
  interval = 10000
  execute_on = 'timestep_end final'
  exodus = true
[]
