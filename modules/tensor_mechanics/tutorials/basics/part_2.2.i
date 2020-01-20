#Tensor Mechanics tutorial: the basics
#Step 2, part 2
#2D axisymmetric RZ simulation of uniaxial tension with finite strain elasticity

[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = necking_quad4.e
  uniform_refine = 1
  second_order = true
[]

[Modules/TensorMechanics/Master]
  [./block1]
    strain = FINITE #change to use finite strain instead of small linearized strain class
    add_variables = true #detects the change of the mesh to second order and automatically sets the variables
    generate_output = 'stress_zz vonmises_stress'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
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
  end_time = 5

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart'
  petsc_options_value = 'asm lu 1 101'
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
