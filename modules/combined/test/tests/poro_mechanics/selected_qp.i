# A sample is unconstrained and its boundaries are
# also impermeable.  Fluid is pumped into the sample via specifying
# the porepressure at all points, and the
# mean stress is monitored at quadpoints in the sample
# This is just to check that the selected_qp in RankTwoScalarAux is working

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
  displacements = 'disp_x disp_y disp_z'
  porepressure = porepressure
  block = 0
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./porepressure]
  [../]
[]

[BCs]
  [./pbdy]
    type = FunctionDirichletBC
    variable = porepressure
    function = 'x*t'
    boundary = 'left right'
  [../]

[]


[Kernels]
  [./grad_stress_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./grad_stress_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
  [./grad_stress_z]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
  [../]
  [./poro_x]
    type = PoroMechanicsCoupling
    variable = disp_x
    component = 0
  [../]
  [./poro_y]
    type = PoroMechanicsCoupling
    variable = disp_y
    component = 1
  [../]
  [./poro_z]
    type = PoroMechanicsCoupling
    variable = disp_z
    component = 2
  [../]
  [./poro_timederiv]
    type = PoroFullSatTimeDerivative
    variable = porepressure
  [../]
[]

[AuxVariables]
  [./mean_stress0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mean_stress1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mean_stress2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mean_stress3]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mean_stress4]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mean_stress5]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mean_stress6]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mean_stress7]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./mean_stress0]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = mean_stress0
    scalar_type = Hydrostatic
    selected_qp = 0
  [../]
  [./mean_stress1]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = mean_stress1
    scalar_type = Hydrostatic
    selected_qp = 1
  [../]
  [./mean_stress2]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = mean_stress2
    scalar_type = Hydrostatic
    selected_qp = 2
  [../]
  [./mean_stress3]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = mean_stress3
    scalar_type = Hydrostatic
    selected_qp = 3
  [../]
  [./mean_stress4]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = mean_stress4
    scalar_type = Hydrostatic
    selected_qp = 4
  [../]
  [./mean_stress5]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = mean_stress5
    scalar_type = Hydrostatic
    selected_qp = 5
  [../]
  [./mean_stress6]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = mean_stress6
    scalar_type = Hydrostatic
    selected_qp = 6
  [../]
  [./mean_stress7]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = mean_stress7
    scalar_type = Hydrostatic
    selected_qp = 7
  [../]
[]



[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '0.0 1.0'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./poro_material]
    type = PoroFullSatMaterial
    porosity0 = 0.1
    biot_coefficient = 1.0
    solid_bulk_compliance = 0.5
    fluid_bulk_compliance = 0.3
    constant_porosity = false
  [../]
[]

[Postprocessors]
  [./mean0]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = mean_stress0
  [../]
  [./mean1]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = mean_stress1
  [../]
  [./mean2]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = mean_stress2
  [../]
  [./mean3]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = mean_stress3
  [../]
  [./mean4]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = mean_stress4
  [../]
  [./mean5]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = mean_stress5
  [../]
  [./mean6]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = mean_stress6
  [../]
  [./mean7]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = mean_stress7
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres asm lu NONZERO 1E-14 1E-10 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = 0
  end_time = 1
  dt = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = false
  file_base = selected_qp
  [./csv]
    type = CSV
  [../]
[]
