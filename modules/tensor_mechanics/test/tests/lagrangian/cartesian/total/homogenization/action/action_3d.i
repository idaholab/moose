# 3D mixed test

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [base]
    type = FileMeshGenerator
    file = '3d.exo'
  []

  [sidesets]
    type = SideSetsFromNormalsGenerator
    input = base
    normals = '-1 0 0
                1 0 0
                0 -1 0
                0 1 0
            '
              '    0 0 -1
                0 0 1'
    fixed_normal = true
    new_boundary = 'left right bottom top back front'
  []
[]

[Modules]
  [TensorMechanics]
    [Master]
      [all]
        strain = FINITE
        add_variables = true
        new_system = true
        formulation = TOTAL
        volumetric_locking_correction = false
        constraint_types = 'stress strain strain strain stress strain strain strain strain'
        targets = 'stress11 strain21 strain31 strain12 stress22 strain32 strain13 strain23 strain33'
        generate_output = 'pk1_stress_xx pk1_stress_xy pk1_stress_xz pk1_stress_yx pk1_stress_yy '
                          'pk1_stress_yz pk1_stress_zx pk1_stress_zy pk1_stress_zz '
                          'deformation_gradient_xx deformation_gradient_xy deformation_gradient_xz '
                          'deformation_gradient_yx deformation_gradient_yy deformation_gradient_yz '
                          'deformation_gradient_zx deformation_gradient_zy deformation_gradient_zz'
      []
    []
  []
[]

[Functions]
  [stress11]
    type = ParsedFunction
    expression = '120.0*t'
  []
  [stress22]
    type = ParsedFunction
    expression = '65*t'
  []
  [strain33]
    type = ParsedFunction
    expression = '8.0e-2*t'
  []
  [strain23]
    type = ParsedFunction
    expression = '2.0e-2*t'
  []
  [strain13]
    type = ParsedFunction
    expression = '-7.0e-2*t'
  []
  [strain12]
    type = ParsedFunction
    expression = '1.0e-2*t'
  []
  [strain32]
    type = ParsedFunction
    expression = '1.0e-2*t'
  []
  [strain31]
    type = ParsedFunction
    expression = '2.0e-2*t'
  []
  [strain21]
    type = ParsedFunction
    expression = '-1.5e-2*t'
  []
  [zero]
    type = ConstantFunction
    value = 0
  []
[]

[BCs]
  [Periodic]
    [x]
      variable = disp_x
      auto_direction = 'x y z'
    []
    [y]
      variable = disp_y
      auto_direction = 'x y z'
    []
    [z]
      variable = disp_z
      auto_direction = 'x y z'
    []
  []

  [fix1_x]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_x
    value = 0
  []
  [fix1_y]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_y
    value = 0
  []
  [fix1_z]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_z
    value = 0
  []

  [fix2_x]
    type = DirichletBC
    boundary = "fix_xy"
    variable = disp_x
    value = 0
  []
  [fix2_y]
    type = DirichletBC
    boundary = "fix_xy"
    variable = disp_y
    value = 0
  []

  [fix3_z]
    type = DirichletBC
    boundary = "fix_z"
    variable = disp_z
    value = 0
  []
[]

[Materials]
  [elastic_tensor_1]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
    poissons_ratio = 0.3
    block = '1'
  []
  [elastic_tensor_2]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 120000.0
    poissons_ratio = 0.21
    block = '2'
  []
  [elastic_tensor_3]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 80000.0
    poissons_ratio = 0.4
    block = '3'
  []
  [elastic_tensor_4]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 76000.0
    poissons_ratio = 0.11
    block = '4'
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
    large_kinematics = true
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 20
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 0.2
  dtmin = 0.2
  end_time = 1.0
[]

[Outputs]
  [out]
    type = Exodus
    file_base = '3d'
  []
[]
