[GlobalParams]
  displacements = 'disp_r'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Modules]
  [TensorMechanics]
    [Master]
      [all]
        formulation = TOTAL
        strain = FINITE
        add_variables = true
        new_system = true
        volumetric_locking_correction = true
      []
    []
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    preset = false
    variable = disp_r
    boundary = left
    value = 0.0
  []
  [top]
    type = FunctionDirichletBC
    preset = false
    variable = disp_r
    boundary = right
    function = 't'
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000.0
    poissons_ratio = 0.25
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
    large_kinematics = true
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10

  dt = 0.1
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
