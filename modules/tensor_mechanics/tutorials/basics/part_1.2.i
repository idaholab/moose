#Tensor Mechanics tutorial: the basics
#Step 1, part 2
#2D simulation of uniaxial tension with linear elasticity with visualized stress

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = necking_quad4.e
  uniform_refine = 1
[]

[Modules/TensorMechanics/Master]
  [./block1]
    strain = SMALL
    add_variables = true
    generate_output = 'stress_xx vonmises_stress' #automatically creates the auxvariables and auxkernels
                                                  #needed to output these stress quanities
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
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./top]
    type = DirichletBC
    variable = disp_y
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
