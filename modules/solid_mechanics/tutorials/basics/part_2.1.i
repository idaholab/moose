#Tensor Mechanics tutorial: the basics
#Step 2, part 1
#2D axisymmetric RZ simulation of uniaxial tension linear elasticity

[GlobalParams]
  displacements = 'disp_r disp_z' #change the variable names for the coordinate system
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = necking_quad4.e
  uniform_refine = 1
[]

[Modules/TensorMechanics/Master]
  [./block1]
    strain = SMALL #detects the change in coordinate system and automatically sets the correct strain class
    add_variables = true
    generate_output = 'stress_zz vonmises_stress' #use stress_zz to get stress_theta quantity
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = disp_r #change the variable to reflect the new displacement names
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_z #change the variable to reflect the new displacement names
    boundary = bottom
    value = 0.0
  [../]
  [./top]
    type = DirichletBC
    variable = disp_z #change the variable to reflect the new displacement names
    boundary = top
    value = 0.0035
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart'
  petsc_options_value = 'asm lu 1 101'
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
