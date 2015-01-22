#The purpose of this test is to test the loadstep predictor.  This is a very
#small, monotonically loaded block of material.  If things are working right,
#the predictor should come very close to exactly nailing the solution on steps
#after the first step.  Because of nonlinear geometry, the predictor is slightly
#off in general, but that is mitigated by setting this up so that the elements
#undergo no rotations.

#The main thing to check here is that once the predictor kicks in, there are
#no iterations required.  Only the last step is output because we don't want
#to check the number of nonlinear iterations in the step where the solver actually
#has to work.

[Mesh]
#  generated =true
  displacements = 'disp_x disp_y disp_z'
#  [./Generation]
#    dim = 3
#    nx = 2
#    ny = 2
#    nz = 1
#    xmin = 0
#    xmax = 2
#    ymin = 0
#    ymax = 2
#    zmin = 0
#    zmax = 1
#    elem_type = HEX8
#  [../]
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
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-7

#  l_max_its = 20

  start_time = 0.0
  dt = 0.25
  num_steps = 4
  end_time = 1.0

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]
[] # Executioner

[Postprocessors]
  [./nonlinear_its]
    type = NumNonlinearIterations
  [../]
[]

[Outputs]
  file_base = out
  interval = 4
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[] # Outputs
