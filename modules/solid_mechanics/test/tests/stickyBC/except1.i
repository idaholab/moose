# Exception testing of StickyBC.  Here min_value > max_value
[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
  [../]
[]

[BCs]
  [./obstruction]
    type = StickyBC
    variable = disp_y
    boundary = bottom
    min_value = 1
    max_value = -1
  [../]
[]

[Materials]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.0
    poissons_ratio = 0.2
  [../]
[]

[Executioner]
  type = Transient
[]
