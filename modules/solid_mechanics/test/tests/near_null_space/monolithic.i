# Plane strain elasticity of a plate that is fixed at its left edge and pulled at its right edge.
# The algebraic multigrid preconditioner of the whole Jacobian receives the rigid body modes of
# the displacements as the near nullspace.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 4
  xmax = 2
[]

[Problem]
  rigid_body_near_null_space_displacements = 'disp_x disp_y'
[]

[Physics/SolidMechanics/QuasiStatic/all]
  strain = SMALL
  add_variables = true
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0
  []
  [pull]
    type = NeumannBC
    variable = disp_x
    boundary = right
    value = 1
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options = '-ksp_view'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'gamg'
[]
