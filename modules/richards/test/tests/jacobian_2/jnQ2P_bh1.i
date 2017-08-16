# quick two phase with production borehole

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


[UserObjects]
  [./DensityWater]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 0.5 # notice small quantity, so the PETSc constant state works
  [../]
  [./DensityGas]
    type = RichardsDensityConstBulk
    dens0 = 0.5
    bulk_mod = 0.3 # notice small quantity, so the PETSc constant state works
  [../]
  [./RelPermWater]
    type = RichardsRelPermPower
    simm = 0.2
    n = 2
  [../]
  [./RelPermGas]
    type = Q2PRelPermPowerGas
    simm = 0.1
    n = 3
  [../]
  [./borehole_total_outflow_mass]
    type = RichardsSumQuantity
  [../]
[]

[Variables]
  [./pp]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = RandomIC
      block = 0
      min = -1
      max = 1
    [../]
  [../]
  [./sat]
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


[Q2P]
  porepressure = pp
  saturation = sat
  water_density = DensityWater
  water_relperm = RelPermWater
  water_viscosity = 1
  gas_density = DensityGas
  gas_relperm = RelPermGas
  gas_viscosity = 1
  diffusivity = 0
[]


[DiracKernels]
  [./bh_water]
    type = Q2PBorehole
    bottom_pressure = -2
    point_file = jn30.bh
    SumQuantityUO = borehole_total_outflow_mass
    variable = sat
    unit_weight = '0 0 0'
    character = 1E12
    fluid_density = DensityWater
    fluid_relperm = RelPermWater
    other_var = pp
    var_is_porepressure = false
    fluid_viscosity = 0.5
  [../]
  [./bh_gas]
    type = Q2PBorehole
    bottom_pressure = -1.5
    point_file = jn30.bh
    SumQuantityUO = borehole_total_outflow_mass
    variable = pp
    unit_weight = '0 0 0'
    character = 1E12
    fluid_density = DensityGas
    fluid_relperm = RelPermGas
    other_var = sat
    var_is_porepressure = true
    fluid_viscosity = 0.25
  [../]
[]



[Materials]
  [./rock]
    type = Q2PMaterial
    block = 0
    mat_porosity = 1E-12 # just so we get virtually no contributions from the time derivatives
    mat_permeability = '1.1E-20 0 0  0 2.2E-20 0  0 0 3.3E-20'
    gravity = '1 2 3'
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    #petsc_options = '-snes_test_display'
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
  file_base = jnQ2P_bh1
  exodus = false
[]
