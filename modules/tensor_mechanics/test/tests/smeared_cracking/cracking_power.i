#
# Simple test of power law softening law for smeared cracking.
# Upon reaching the failure stress in the x direction, the
# softening model abruptly reduces the stress to a fraction
# of its original value, and re-loading occurs at a reduced
# stiffness. This is repeated multiple times.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./displ]
    type = PiecewiseLinear
    x = '0 1 2 3  4'
    y = '0 1 0 -1 0'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
  [../]
[]

[BCs]
  [./pull]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = displ
  [../]
  [./left]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./back]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.8e7
    poissons_ratio = 0
  [../]
  [./elastic_stress]
    type = ComputeSmearedCrackingStress
    cracking_stress = 1.68e6
    softening_models = power_law_softening
  [../]
  [./power_law_softening]
    type = PowerLawSoftening
    stiffness_reduction = 0.3333
  [../]
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type'
  petsc_options_value = '101                asm      lu'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  l_tol = 1e-5
  start_time = 0.0
  end_time = 1.0
  dt = 0.01
[]

[Outputs]
  exodus = true
[]
