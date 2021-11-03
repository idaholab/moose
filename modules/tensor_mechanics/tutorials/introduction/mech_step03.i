#
# Added subdomains and subdomain-specific properties
# https://mooseframework.inl.gov/modules/tensor_mechanics/tutorials/introduction/step03.html
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 40
    ny = 20
    xmax = 2
    ymax = 1
  []

  # assign two subdomains
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = generated
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '1 1 0'
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    input = block1
    block_id = 2
    bottom_left = '1 0 0'
    top_right = '2 1 0'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [Pressure]
    [top]
      boundary = top
      function = 1e7*t
    []
  []
[]

[Materials]
  [elasticity1]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e9
    poissons_ratio = 0.3
    block = 1
  []
  [elasticity2]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 5e8
    poissons_ratio = 0.3
    block = 2
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  end_time = 5
  dt = 1
[]

[Outputs]
  exodus = true
[]
