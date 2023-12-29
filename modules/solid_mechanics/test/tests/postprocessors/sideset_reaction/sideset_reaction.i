[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
[]

[Postprocessors]
  [./react_x]
    type = SidesetReaction
    direction = '1 0 0'
    stress_tensor = stress
    boundary = right
  [../]
[]

[Modules/TensorMechanics/Master]
  [plane_strain]
    strain = FINITE
    extra_vector_tags = 'ref'
    add_variables = true
  []
[]

[BCs]
  [leftx]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0.0
  []
  [bottomy]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
  [backz]
    type = DirichletBC
    boundary = back
    variable = disp_z
    value = 0.0
  []
  [rightx]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_x
    function = 't'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient
  line_search = none

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10

  # time control
  start_time = 0.0
  dt = 0.01
  dtmin = 0.01
  end_time = 0.2
[]

[Outputs]
  csv = true
[]
