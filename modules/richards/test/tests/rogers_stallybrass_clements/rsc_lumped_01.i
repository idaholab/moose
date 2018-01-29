# RSC test with high-res time and spatial resolution
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 600
  ny = 1
  xmin = 0
  xmax = 10 # x is the depth variable, called zeta in RSC
  ymin = 0
  ymax = 0.05
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
  density_UO = 'DensityWater DensityOil'
  relperm_UO = 'RelPerm RelPerm'
  SUPG_UO = 'SUPGstandard SUPGstandard'
  sat_UO = 'Saturation Saturation'
  seff_UO = 'SeffWater SeffOil'
[]

[Functions]
  [./dts]
    type = PiecewiseLinear
    y = '3E-3 3E-2 0.05'
    x = '0 1 5'
  [../]
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = 'pwater poil'
  [../]
  [./DensityWater]
    type = RichardsDensityConstBulk
    dens0 = 10
    bulk_mod = 2E9
  [../]
  [./DensityOil]
    type = RichardsDensityConstBulk
    dens0 = 20
    bulk_mod = 2E9
  [../]
  [./SeffWater]
    type = RichardsSeff2waterRSC
    oil_viscosity = 2E-3
    scale_ratio = 2E3
    shift = 10
  [../]
  [./SeffOil]
    type = RichardsSeff2gasRSC
    oil_viscosity = 2E-3
    scale_ratio = 2E3
    shift = 10
  [../]
  [./RelPerm]
    type = RichardsRelPermMonomial
    simm = 0
    n = 1
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0.0
    sum_s_res = 0.0
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 1.0E-2
  [../]
[]


[Variables]
  [./pwater]
  [../]
  [./poil]
  [../]
[]

[ICs]
  [./water_init]
    type = ConstantIC
    variable = pwater
    value = 0
  [../]
  [./oil_init]
    type = ConstantIC
    variable = poil
    value = 15
  [../]
[]

[Kernels]
  [./richardstwater]
    type = RichardsLumpedMassChange
    variable = pwater
  [../]
  [./richardsfwater]
    type = RichardsFlux
    variable = pwater
  [../]
  [./richardstoil]
    type = RichardsLumpedMassChange
    variable = poil
  [../]
  [./richardsfoil]
    type = RichardsFlux
    variable = poil
  [../]
[]


[AuxVariables]
  [./SWater]
  [../]
  [./SOil]
  [../]
[]


[AuxKernels]
  [./Seff1VGwater_AuxK]
    type = RichardsSeffAux
    variable = SWater
    seff_UO = SeffWater
    pressure_vars = 'pwater poil'
  [../]
  [./Seff1VGoil_AuxK]
    type = RichardsSeffAux
    variable = SOil
    seff_UO = SeffOil
    pressure_vars = 'pwater poil'
  [../]
[]


[BCs]
# we are pumping water into a system that has virtually incompressible fluids, hence the pressures rise enormously.  this adversely affects convergence because of almost-overflows and precision-loss problems.  The fixed things help keep pressures low and so prevent these awful behaviours.   the movement of the saturation front is the same regardless of the fixed things.
  active = 'recharge fixedoil fixedwater'
  [./recharge]
    type = RichardsPiecewiseLinearSink
    variable = pwater
    boundary = 'left'
    pressures = '-1E10 1E10'
    bare_fluxes = '-1 -1'
    use_mobility = false
    use_relperm = false
  [../]
  [./fixedwater]
    type = DirichletBC
    variable = pwater
    boundary = 'right'
    value = 0
  [../]
  [./fixedoil]
    type = DirichletBC
    variable = poil
    boundary = 'right'
    value = 15
  [../]
[]


[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.25
    mat_permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
    viscosity = '1E-3 2E-3'
    gravity = '0E-0 0 0'
    linear_shape_fcns = true
  [../]
[]

[Preconditioning]
  active = 'andy'

  [./andy]
    type = SMP
    full = true
    petsc_options = ''
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10000'
  [../]
[]


[Executioner]
  type = Transient
  solve_type = Newton
  petsc_options = '-snes_converged_reason'
  end_time = 5

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]


[Outputs]
  file_base = rsc_lumped_01
  interval = 100000
  execute_on = 'initial final'
  exodus = true
[]
