#
# We study the effects of volumetric locking
# https://mooseframework.inl.gov/modules/tensor_mechanics/tutorials/introduction/answer04b.html
#

[GlobalParams]
  displacements = 'disp_x disp_y'
  # elem_type applies to the GeneratedMeshGenerator blocks
  elem_type = QUAD4
  # volumetric_locking_correction applies to the TensorMechanics Master action
  volumetric_locking_correction = false
  # uniform_refine applies to the final mesh
  uniform_refine = 0
[]

[Mesh]
  [generated1]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 15
    xmin = -0.6
    xmax = -0.1
    ymax = 5
    bias_y = 0.9
    boundary_name_prefix = pillar1
  []
  [generated2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 15
    xmin = 0.1
    xmax = 0.6
    ymax = 5
    bias_y = 0.9
    boundary_name_prefix = pillar2
    boundary_id_offset = 4
  []
  [collect_meshes]
    type = MeshCollectionGenerator
    inputs = 'generated1 generated2'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'pillar1_bottom pillar2_bottom'
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'pillar1_bottom pillar2_bottom'
    value = 0
  []
  [Pressure]
    [sides]
      boundary = 'pillar1_left pillar2_right'
      function = 1e4*t
    []
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e9
    # near incopmpressible material
    poissons_ratio = 0.49
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Postprocessors]
  [x_deflection]
    type = NodalExtremeValue
    value_type = max
    variable = disp_x
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = none
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  end_time = 5
  dt = 0.5
  [Predictor]
    type = SimplePredictor
    scale = 1
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
