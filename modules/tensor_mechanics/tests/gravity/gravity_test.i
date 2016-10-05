#
# Pressure Test
#
# This test is designed to apply a gravity body force.
#
# The mesh is composed of one block with a single element.
# The bottom is fixed in all three directions.  Poisson's ratio
# is zero and the density is 20/9.81
# which makes it trivial to check displacements.
#

[Mesh]
  type = GeneratedMesh
  dim = 3
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./gravity_y]
    type = Gravity
    variable = disp_y
    value = -9.81
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]
[]

[Materials]
  [./Elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0 0.5e6'
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./density]
    type = GenericConstantMaterial
    block = 0
    prop_names = current_density
    prop_values = 2.0387
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-10
  l_max_its = 20
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]

