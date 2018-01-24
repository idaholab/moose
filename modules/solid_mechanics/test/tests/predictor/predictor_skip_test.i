# The purpose of this test is to test the simple predictor.  This is a very
# small, monotonically loaded block of material.  If things are working right,
# the predictor should come very close to exactly nailing the solution on steps
# after the first step.  Because of nonlinear geometry, the predictor is slightly
# off in general, but that is mitigated by setting this up so that the elements
# undergo no rotations.

# This test checks to see that the predictor is skipped in the last step.

[Mesh]
  displacements = 'disp_x disp_y disp_z'
  file = predictor_test.e
[] # Mesh

[Functions]
  [./ramp1]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = 0.2
  [../]
[] # Functions

[Variables]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]

[] # Variables

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[BCs]

  [./ss1_x]
    type = PresetBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]
  [./ss1_y]
    type = PresetBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]
  [./ss1_z]
    type = PresetBC
    variable = disp_z
    boundary = 4
    value = 0.0
  [../]

  [./ss2_x]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 2
    function = ramp1
  [../]
  [./ss2_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 2
    function = ramp1
  [../]
  [./ss2_z]
    type = PresetBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]

[] # BCs

[Materials]

  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
[] # Materials

[Executioner]

  type = Transient


  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'




# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-8

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-9

#  l_max_its = 20

  start_time = 0.0
  dt = 0.5
  num_steps = 2
  end_time = 1.0

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
    skip_times = '1.0'
  [../]
[] # Executioner

[Postprocessors]
  [./initial_residual]
    type = Residual
    residual_type = initial_after_preset
  [../]
[]

[Outputs]
  csv = true
  exodus = true
[] # Outputs
