[Mesh]
  use_displaced_mesh = false
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
  [A]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '0 0 0'
    top_right = '1 1 1'
    block_id = 0
    block_name = A
  []
  [B]
    type = SubdomainBoundingBoxGenerator
    input = A
    bottom_left = '0 0 0'
    top_right = '0.25 0.25 0.5'
    block_id = 1
    block_name = B
  []
[]

[Variables]
  [x]
    block = 'B'
  []
[]

[Modules]
  [TensorMechanics]
    [Master]
      displacements = 'disp_x disp_y disp_z'
      [all]
        displacements = 'disp_x disp_y disp_z'
        strain = FINITE
        add_variables = true
        new_system = true
        formulation = TOTAL
        volumetric_locking_correction = true
        block = 'A'
        constraint_types = 'stress strain strain stress stress strain stress stress strain'
        targets = '0 0 0 0 0 0 0 0 0'
      []
    []
  []
[]

[Materials]
  [stress]
    type = ComputeLagrangianLinearElasticStress
    block = 'A'
  []
  [C1]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e5
    poissons_ratio = 0.3
    block = 'A'
  []
[]

[Kernels]
  [blah]
    type = NullKernel
    variable = x
  []
[]

[Executioner]
  type = Steady
[]
