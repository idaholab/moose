#Tensor Mechanics tutorial: the basics
#Step 2, part 3
#2D axisymmetric RZ simulation of uniaxial tension with J2 plasticity with no
#hardening

[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = necking_quad4.e
  uniform_refine = 0
  second_order = true
[]

[Modules/TensorMechanics/Master]
  [./block1]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_yy strain_yy' #use the yy option to get the zz component in axisymmetric coords
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
  [./str]
    type = TensorMechanicsHardeningConstant
    value = 2.4e2
  [../]
  [./J2]
    type = TensorMechanicsPlasticJ2
    yield_strength = str
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-9
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = disp_r
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]
  [./top]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = top
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
  end_time = 20

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart'
  petsc_options_value = 'asm lu 1 101'
[]

[Postprocessors]
  [./ave_stress_bottom]
    type = SideAverageValue
    variable = stress_yy
    boundary = bottom
  [../]
  [./ave_strain_bottom]
    type = SideAverageValue
    variable = strain_yy
    boundary = bottom
  [../]
[]

[Outputs]
  exodus = true
  perf_graph = true
  csv = true
  print_linear_residuals = false
[]
