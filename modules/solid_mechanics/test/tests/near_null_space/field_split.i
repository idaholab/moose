# Elasticity of a cube that is fixed at its left face and pulled at its right face, and an
# uncoupled reaction-diffusion equation. The field split preconditioner solves the displacement
# block with algebraic multigrid, which receives the rigid body modes of the displacements as the
# near nullspace of that block.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 3
  nz = 3
[]

[Problem]
  rigid_body_near_null_space_displacements = 'disp_x disp_y disp_z'
[]

# Declaring the displacements consecutively with the same type puts them in one libMesh variable
# group, which numbers the displacements of each node consecutively
[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [T]
  []
[]

[Physics/SolidMechanics/QuasiStatic/all]
  strain = SMALL
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = T
  []
  [reaction]
    type = Reaction
    variable = T
  []
  [source]
    type = BodyForce
    variable = T
  []
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
  [fix_z]
    type = DirichletBC
    variable = disp_z
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

[Preconditioning]
  [fsp]
    type = FSP
    topsplit = 'disp_T'
    [disp_T]
      splitting = 'disp temperature'
    []
    [disp]
      vars = 'disp_x disp_y disp_z'
      petsc_options_iname = '-pc_type'
      petsc_options_value = 'gamg'
    []
    [temperature]
      vars = 'T'
      petsc_options_iname = '-pc_type'
      petsc_options_value = 'jacobi'
    []
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options = '-ksp_view'
[]
