[Mesh]
  type = GeneratedMesh
  dim = 1
  elem_type = EDGE
  nx = 1
[]

[GlobalParams]
  displacements = 'disp_x'
[]

[Variables]
  [disp_x]
  []
[]

[Kernels]
  [solid]
    type = StressDivergenceTruss
    component = 0
    variable = disp_x
  []
[]

[AuxVariables]
  [area]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [area]
    type = ConstantAux
    variable = area
    value = 1
    execute_on = 'initial timestep_begin'
  []
[]

[BCs]
  [fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
[]

[Materials]
  [elasticity]
    type = ComputeElasticityTruss
    youngs_modulus = 1e6
  []
  [strain]
    type = ComputeIncrementalTrussStrain
    displacements = 'disp_x'
    area = area
    eigenstrain_names = 'thermal'
  []
  [stress]
    type = ComputeTrussResultants
    area = area
  []
  [thermal]
    type = ComputeThermalExpansionEigenstrainTruss
    thermal_expansion_coeff = 1e-4
    temperature = 100
    stress_free_temperature = 0
    eigenstrain_name = thermal
  []
[]

[Postprocessors]
  [disp_x]
    type = PointValue
    point = '1.0 0.0 0.0'
    variable = disp_x
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-11
  l_max_its = 20
  dt = 1
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
