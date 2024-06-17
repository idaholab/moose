# One element test to test the central difference time integrator in 3D.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 2
  xmin = 0.0
  xmax = 1
  ymin = 0.0
  ymax = 1
  zmin = 0.0
  zmax = 2
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[AuxVariables]
  [vel_x]
  []
  [accel_x]
  []
  [vel_y]
  []
  [accel_y]
  []
  [vel_z]
  []
  [accel_z]
  []
[]

[AuxKernels]
  [accel_x]
    type = TestNewmarkTI
    variable = accel_x
    displacement = disp_x
    first = false
  []
  [vel_x]
    type = TestNewmarkTI
    variable = vel_x
    displacement = disp_x
  []
  [accel_y]
    type = TestNewmarkTI
    variable = accel_y
    displacement = disp_y
    first = false
  []
  [vel_y]
    type = TestNewmarkTI
    variable = vel_y
    displacement = disp_x
  []
  [accel_z]
    type = TestNewmarkTI
    variable = accel_z
    displacement = disp_z
    first = false
  []
  [vel_z]
    type = TestNewmarkTI
    variable = vel_z
    displacement = disp_z
  []
[]

[Kernels]
  [DynamicSolidMechanics]
    displacements = 'disp_x disp_y disp_z'
  []
  [inertia_x]
    type = InertialForce
    variable = disp_x
  []
  [inertia_y]
    type = InertialForce
    variable = disp_y
  []
  [inertia_z]
    type = InertialForce
    variable = disp_z
  []
[]

[BCs]
  [x_bot]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'back'
    function = dispx
    preset = false
  []
  [y_bot]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'back'
    function = dispy
    preset = false
  []
  [z_bot]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'back'
    function = dispz
    preset = false
  []
  [Periodic]
    [x_dir]
      variable = 'disp_x disp_y disp_z'
      primary = 'left'
      secondary = 'right'
      translation = '1.0 0.0 0.0'
    []
    [y_dir]
      variable = 'disp_x disp_y disp_z'
      primary = 'bottom'
      secondary = 'top'
      translation = '0.0 1.0 0.0'
    []
  []
[]

[Functions]
  [dispx]
    type = PiecewiseLinear
    x = '0.0 1.0 2.0 3.0 4.0' # time
    y = '0.0 1.0 0.0 -1.0 0.0' # displacement
  []
  [dispy]
    type = ParsedFunction
    value = 0.1*t*t*sin(10*t)
  []
  [dispz]
    type = ParsedFunction
    value = 0.1*t*t*sin(20*t)
  []
[]

[Materials]
  [elasticity_tensor_block]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
    block = 0
  []
  [strain_block]
    type = ComputeIncrementalStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
    implicit = false
  []
  [stress_block]
    type = ComputeFiniteStrainElasticStress
    block = 0
  []
  [density]
    type = GenericConstantMaterial
    block = 0
    prop_names = density
    prop_values = 1e4
  []
  [density_scaling]
    type = DensityScaling
    block = 0
    density = density
    desired_time_step = 0.06
    output_properties = density_scaling
    outputs = 'exodus'
    factor = 0.5
  []
[]

[Executioner]
  type = Transient
  start_time = -0.01
  end_time = 0.1
  dt = 0.005
  timestep_tolerance = 1e-6
  [TimeIntegrator]
    type = CentralDifference
    use_constant_mass = false
    solve_type = lumped
  []
  [TimeStepper]
    type = PostprocessorDT
    postprocessor = time_step
  []
[]

[Postprocessors]
  [accel_6x]
    type = NodalVariableValue
    nodeid = 6
    variable = accel_x
  []
  [time_step]
    type = CriticalTimeStep
    factor = 0.5
    density = density
    density_scaling = density_scaling
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
