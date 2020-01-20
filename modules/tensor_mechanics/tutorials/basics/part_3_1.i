#Tensor Mechanics tutorial: the basics
#Step 3, part 1
#3D simulation of uniaxial tension with J2 plasticity

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [file_mesh]
    type = FileMeshGenerator
    file = necking_quad4.e
  []
  [extrude]
    type = MeshExtruderGenerator
    extrusion_vector = '0 0 0.5'
    num_layers = 2
    bottom_sideset = 'back'
    top_sideset = 'front'
    input = file_mesh
  []
  uniform_refine = 0
  second_order = true
[]

[Modules/TensorMechanics/Master]
  [./block1]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_yy strain_yy'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1e-9
    plastic_models = J2
  [../]
[]

[UserObjects]
  [./hardening]
    type = TensorMechanicsHardeningCubic
    value_0 = 2.4e2
    value_residual = 3.0e2
    internal_0 = 0
    internal_limit = 0.005
  [../]
  [./J2]
    type = TensorMechanicsPlasticJ2
    yield_strength = hardening
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-9
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = disp_x #change the variable to reflect the new displacement names
    boundary = 1
    value = 0.0
  [../]
  [./back]
    type = DirichletBC
    variable = disp_z #change the variable to reflect the new displacement names
    boundary = back
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_y #change the variable to reflect the new displacement names
    boundary = 3
    value = 0.0
  [../]
  [./top]
    type = FunctionDirichletBC
    variable = disp_y #change the variable to reflect the new displacement names
    boundary = 4
    function = '0.0007*t'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.25
  end_time = 16

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart'
  petsc_options_value = 'asm lu 1 101'
[]

[Postprocessors]
  [./ave_stress_bottom]
    type = SideAverageValue
    variable = stress_yy
    boundary = 3
  [../]
  [./ave_strain_bottom]
    type = SideAverageValue
    variable = strain_yy
    boundary = 3
  [../]
[]

[Outputs]
  exodus = true
  perf_graph = true
  csv = true
  print_linear_residuals = false
[]
