#
# Gravity Test
#
# This test is designed to apply a gravity body force.
#
# The mesh is composed of one block with a single element.
# The bottom is fixed in all three directions.  Poisson's ratio
# is zero and the density is 20/9.81
# which makes it trivial to check displacements.
#

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
  []
[]

[Kernels]
  [gravity_y]
    type = Gravity
    variable = disp_y
    value = -9.81
  []
[]

[BCs]
  [no_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [no_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 0.5e6'
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 2.0387
  []
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-10
  l_max_its = 20
[]

[Outputs]
  [out]
    type = Exodus
    elemental_as_nodal = true
  []
[]
