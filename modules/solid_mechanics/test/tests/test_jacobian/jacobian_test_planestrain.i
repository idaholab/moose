# This test is designed to test the jacobian for a single
# element with volumetric locking correction.
# This test uses small plane strain formulations.

# To test the jacobian obtained from finite strain formulation comment out
# formulation = NonlinearPlaneStrain in the materials block.

# The mesh contains one element whose y displacement is zero at
# the bottom surface (y=0) and -1.0 at the top surface (y=1).

# Result: The hand coded jacobian matches well with the finite
# difference jacobian with a error norm of 2.9e-15 for the small planestrain
# formulation.

# For the finite strain formulation, the error norm is in the order of 2.3e-8.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [./x_disp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./y_disp]
    order = FIRST
    family = LAGRANGE
  [../]

[]

[SolidMechanics]
  [./solid]
    disp_x = x_disp
    disp_y = y_disp
  [../]
[]

[BCs]

  [./y_force]
    type = NeumannBC
    variable = y_disp
    boundary = top
    value = -1.0
  [../]

  [./bottom]
    type = DirichletBC
    variable = y_disp
    boundary = bottom
    value = 0.0
  [../]

[]

[Materials]
  [./elastic]
    type = Elastic
    block = 0
    disp_x = x_disp
    disp_y = y_disp
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    formulation = PlaneStrain
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

[Outputs]
  exodus = true
[]
