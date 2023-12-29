[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
  stabilize_strain = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
  use_displaced_mesh = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
    save_in = 'ry'
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
[]

[AuxVariables]
  [ry]
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
  []
  [back_z]
    type = DirichletBC
    boundary = back
    variable = disp_z
    value = 0
  []
  [disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = 't'
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000.0
    poissons_ratio = 0.25
  []
  [strain]
    type = ComputeLagrangianStrain
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  end_time = 0.1

  solve_type = 'newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
[]

[Postprocessors]
  [Ry]
    type = NodalSum
    variable = ry
    boundary = top
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
[]
