left_cube_refinement = 2
right_cube_refinement = 2

[Mesh]
  [./left_cube]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${left_cube_refinement}
    ny = ${left_cube_refinement}
    elem_type = QUAD4
  [../]
  [./right_cube]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${right_cube_refinement}
    ny = ${right_cube_refinement}
    elem_type = QUAD4
  [../]
  [./combined]
    type = CombinerGenerator
    inputs = 'left_cube right_cube'
    positions = '0 0 0
                 1.1 0 0'
  [../]
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Modules/TensorMechanics/Master/action]
  add_variables = true
  strain = FINITE
  generate_output = 'vonmises_stress'
  # volumetric_locking_correction = true
[]


[BCs]
  [./right_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = 0
  [../]
  [./right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = t
  [../]
[]

[Materials]
  [./plank]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    # type = ComputeLinearElasticStress
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_converged_reason'
  l_max_its = 100
  end_time = 0.1
  dtmin = 0.1
[]

[Outputs]
  exodus = true
[]
