[Mesh]
  [physical_model]
    type = GeneratedMeshGenerator
    dim = 1
    xmax = 10
    subdomain_ids = 1
  []
  [trash_can]
    type = GeneratedMeshGenerator
    dim = 1
    subdomain_ids = 111
    subdomain_name = trash_can
    boundary_name_prefix = trash_can
    boundary_id_offset = 111
  []
  [whole]
    type = MeshCollectionGenerator
    inputs = 'physical_model trash_can'
  []
[]

[GlobalParams]
  displacements = disp_x
  block = 1
[]

[Problem]
  extra_tag_matrices = mass
[]

[Variables]
  [disp_x]
  []
[]

[Kernels]
  [DynamicSolidMechanics]
    incremental = true
  []
  [massmatrix_x]
    type = MassMatrix
    density = 2500
    matrix_tags = mass
    variable = disp_x
  []
[]

[BCs]
  [no_x]
    type = ExplicitDirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0.0
  []
[]

[Materials]
  [elasticity_top_bot]
    type = ComputeIsotropicElasticityTensor
    implicit = false
    youngs_modulus = 15E9
    poissons_ratio = 0.25
  []
  [strain]
    type = ComputeSmallStrain
    implicit = false
  []
  [stress_0]
    type = ComputeLinearElasticStress
    implicit = false
  []
  [density]
    type = GenericConstantMaterial
    implicit = false
    prop_names = density
    prop_values = 2500
  []
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = mass
    second_order_vars = disp_x
    use_constant_mass = false
# LOGAN: note this line. If restrict_to_active_blocks=false then no segfault
restrict_to_active_blocks = true
  []

  dt = 1
  dtmin = 1
  end_time = 1
[]

[Outputs]
  exodus = true
  console = true
[]
