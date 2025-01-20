[Mesh]
  type = FileMesh
  file = pressure_test.e
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [rampConstant]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '1. 1. 1.'
    scale_factor = 1.0
  []
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [SolidMechanics]
    displacements = 'disp_x disp_y disp_z'
  []
[]

[BCs]
  [no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 5
    value = 0.0
  []
  [no_z]
    type = DirichletBC
    variable = disp_z
    boundary = 6
    value = 0.0
  []
  [Pressure]
    [Side1]
      boundary = 1
      function = rampConstant
      displacements = 'disp_x disp_y disp_z'
      control_tags = 'tag_pressure'
    []
    [Side2]
      boundary = 2
      function = rampConstant
      displacements = 'disp_x disp_y disp_z'
      factor = 2.0
      control_tags = 'tag_pressure'
    []
    [Side3]
      boundary = 3
      function = rampConstant
      displacements = 'disp_x disp_y disp_z'
      control_tags = 'tag_pressure'
    []
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 0.5e6
    # To facilitate examining results:
    # deformation only shows in one direction
    poissons_ratio = 0
  []
  [strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Controls]
  # Turn Pressure on some boundaries for step 2, off at step 3
  [pressure_crank]
    type = TimePeriod
    enable_objects = 'BCs/Pressure/Side1'
    disable_objects = 'BCs/Pressure/Side2 BCs/Pressure/Side3'
    # Side1 for initial and second time step
    start_time = 0
    # Then both Side2 and Side3
    end_time = 0.9
    # All Pressure are controlled outside of the period as well
    reverse_on_false = true
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 1.0
  num_steps = 3
  end_time = 2.0
[]

[Outputs]
  [out]
    type = Exodus
    elemental_as_nodal = true
  []
  [controls]
    type = ControlOutput
    show_active_objects = false
  []
[]
