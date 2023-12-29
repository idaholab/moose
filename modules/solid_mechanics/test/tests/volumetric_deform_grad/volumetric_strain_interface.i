#This test has volumetric deformation gradient as identity
#Test the interface
#Results should match with elasticity
[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX8
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = true
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = RankTwoAux
    variable = stress_zz
    rank_two_tensor = stress
    index_j = 2
    index_i = 2
    execute_on = timestep_end
    block = 0
  [../]
[]

[BCs]
  [./symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./tdisp]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = '0.01*t'
  [../]
[]

[Materials]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./volumetric_strain]
    type = ComputeVolumetricDeformGrad
    pre_deform_grad_name = deformation_gradient
    volumetric_deform_grad_name = volumetric_deformation_gradient
    post_deform_grad_name = elastic_deformation_gradient
    block = 0
  [../]
  [./elastic_stress]
    type = ComputeDeformGradBasedStress
    deform_grad_name = elastic_deformation_gradient
    elasticity_tensor_name = elasticity_tensor
    stress_name = elastic_stress
    jacobian_name = elastic_jacobian
    block = 0
  [../]
  [./corrected_stress]
    type = VolumeDeformGradCorrectedStress
    pre_stress_name = elastic_stress
    deform_grad_name = volumetric_deformation_gradient
    pre_jacobian_name = elastic_jacobian
    stress_name = stress
    jacobian_name = Jacobian_mult
    block = 0
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '2.8e5 1.2e5 1.2e5 2.8e5 1.2e5 2.8e5 0.8e5 0.8e5 0.8e5'
    fill_method = symmetric9
  [../]
[]

[Postprocessors]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
    block = 'ANY_BLOCK_ID 0'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.02
  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'
  dtmax = 10.0
  nl_rel_tol = 1e-10
  dtmin = 0.02
  num_steps = 10
[]

[Outputs]
  csv = true
[]
