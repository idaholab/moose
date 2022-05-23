# This test is designed to test the jacobian for a single
# element with/without volumetric locking correction.

# The mesh contains one element whose y displacement is zero at
# the bottom surface (y=0) and -1.0 at the top surface (y=1).

# Result: The hand coded jacobian matches well with the finite
# difference jacobian with an error norm in the order of 1e-15
# for total and incremental small strain formulations and with
# an error in the order of 1e-8 for finite strain formulations.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[BCs]

  [./y_force]
    type = NeumannBC
    variable = disp_y
    boundary = top
    value = -1.0
  [../]

  [./bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]

[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    block = 0
  [../]
  [./strain]
    block = 0
  [../]
  [./stress]
    block = 0
  [../]
[]

[Preconditioning]
  active = 'smp'
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient #Transient

  solve_type = NEWTON
  petsc_options = '-snes_check_jacobian -snes_check_jacobian_view'

  l_max_its = 100
  nl_abs_tol = 1e-4
  start_time = 0.0
  num_steps = 1
  dt = 0.005
  dtmin = 0.005
  end_time = 0.005
[]
