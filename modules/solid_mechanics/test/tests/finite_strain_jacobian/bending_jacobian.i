[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 2
    nx = 10
    ny = 2
    elem_type = QUAD4
  []
  [corner]
    type = ExtraNodesetGenerator
    new_boundary = 101
    coord = '0 0'
    input = generated_mesh
  []
  [side]
    type = ExtraNodesetGenerator
    new_boundary = 102
    coord = '10 0'
    input = corner
  []
  [mid]
    type = ExtraNodesetGenerator
    new_boundary = 103
    coord = '5 2'
    input = side
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    use_finite_deform_jacobian = true
    volumetric_locking_correction = false
  [../]
[]

[Materials]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1.684e5 0.176e5 0.176e5 1.684e5 0.176e5 1.684e5 0.754e5 0.754e5 0.754e5'
  [../]
[]

[BCs]
 [./fix_corner_x]
   type = DirichletBC
   variable = disp_x
   boundary = 101
   value = 0
 [../]
 [./fix_corner_y]
   type = DirichletBC
   variable = disp_y
   boundary = 101
   value = 0
 [../]
 [./fix_y]
   type = DirichletBC
   variable = disp_y
   boundary = 102
   value = 0
 [../]
 [./move_y]
   type = FunctionDirichletBC
   variable = disp_y
   boundary = 103
   function = '-t'
 [../]
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-10
  nl_max_its = 10

  l_tol  = 1e-4
  l_max_its = 50

  dt = 0.1
  dtmin = 0.1

  num_steps = 2
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  exodus = true
[]
