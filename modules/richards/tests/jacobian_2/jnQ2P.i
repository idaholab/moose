# quick two phase

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
    type = RichardsDensityConstBulkCut
    dens0 = 1
    cut_limit = 1.1
    zero_point = -1.1
    bulk_mod = 1.0 # notice small quantity, so the PETSc constant state works
  [../]
  [./DensityGas]
    type = RichardsDensityConstBulk
    dens0 = 0.5
    bulk_mod = 0.5 # notice small quantity, so the PETSc constant state works
  [../]
  [./RelPermWater]
    type = RichardsRelPermPower
    simm = 0.2
    n = 2
  [../]
  [./RelPermWaterCubic]
    type = RichardsRelPermMonomial
    simm = 0.05
    n = 3
  [../]
  [./RelPermGas]
    type = Q2PRelPermPowerGas
    simm = 0.1
    n = 3
  [../]
[]

[Variables]
  [./pp]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = RandomIC
      block = 0
      min = 0
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
  [./nonQ2P_var]
  []
[]

[Q2P]
  porepressure = pp
  saturation = sat
  water_density = DensityWater
  water_relperm = RelPermWater
  water_relperm_for_diffusion = RelPermWaterCubic
  water_viscosity = 1
  gas_density = DensityGas
  gas_relperm = RelPermGas
  gas_viscosity = 1
  diffusivity = 1E-2
[]

[Kernels]
  [./nonQ2P_variable_check]
    type = BodyForce
    variable = nonQ2P_var
    function = 0
  [../]
[]


[Materials]
  [./rock]
    type = Q2PMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1.1 0 0  0 2.2 0  0 0 3.3'
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
  dt = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = jnQ2P
  exodus = false
[]
