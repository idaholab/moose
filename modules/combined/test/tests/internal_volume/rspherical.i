#
# Internal Volume Test
#
# This test is designed to compute the internal volume of a space considering
#   an embedded volume inside.
#
# The mesh is composed of two blocks with an interior cavity of volume 3.
#   The volume of each of the blocks is also 3.  The volume of the entire sphere
#   is 9.
#
[GlobalParams]
  displacements = 'disp_x'
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Mesh]
  file = meshes/rspherical.e
  construct_side_list_from_node_list = true
[]

[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = 1e4
  [../]
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    incremental = true
    strain = FINITE
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = '1 2 3 4'
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1 3'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]

  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 3'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK

  start_time = 0.0
  dt = 1.0
  end_time = 1.0
[]

[Postprocessors]
  [./internalVolume]
    type = InternalVolume
    boundary = 10
    component = 0
    execute_on = 'initial timestep_end'
  [../]
  [./intVol1]
    type = InternalVolume
    boundary = 2
    component = 0
    execute_on = 'initial timestep_end'
  [../]
  [./intVol1Again]
    type = InternalVolume
    boundary = 9
    component = 0
    execute_on = 'initial timestep_end'
  [../]
  [./intVol2]
    type = InternalVolume
    boundary = 11
    component = 0
    execute_on = 'initial timestep_end'
  [../]
  [./intVolTotal]
    type = InternalVolume
    boundary = 4
    component = 0
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  csv = true
[]
