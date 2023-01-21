[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  origin = '0 0 2'
  direction = '0 0 1'
  polar_moment_of_inertia = pmi
  factor = t
[]

[Mesh]
  [ring]
    type = AnnularMeshGenerator
    nr = 1
    nt = 30
    rmin = 0.95
    rmax = 1
  []
  [extrude]
    type = MeshExtruderGenerator
    input = ring
    extrusion_vector = '0 0 2'
    bottom_sideset = 'bottom'
    top_sideset = 'top'
    num_layers = 5
  []
[]

[AuxVariables]
  [alpha_var]
  []
  [shear_stress_var]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [alpha]
    type = RotationAngle
    variable = alpha_var
  []
  [shear_stress]
    type = ParsedAux
    variable = shear_stress_var
    coupled_variables = 'stress_yz stress_xz'
    expression = 'sqrt(stress_yz^2 + stress_xz^2)'
  []
[]

[BCs]
  # fix bottom
  [fix_x]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0
  []
  [fix_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
  []
  [fix_z]
    type = DirichletBC
    boundary = bottom
    variable = disp_z
    value = 0
  []

  # twist top
  [twist_x]
    type = Torque
    boundary = top
    variable = disp_x
  []
  [twist_y]
    type = Torque
    boundary = top
    variable = disp_y
  []
  [twist_z]
    type = Torque
    boundary = top
    variable = disp_z
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = SMALL
    generate_output = 'vonmises_stress stress_yz stress_xz'
  []
[]

[Postprocessors]
  [pmi]
    type = PolarMomentOfInertia
    boundary = top
    # execute_on = 'INITIAL NONLINEAR'
    execute_on = 'INITIAL'
  []
  [alpha]
    type = SideAverageValue
    variable = alpha_var
    boundary = top
  []
  [shear_stress]
    type = ElementAverageValue
    variable = shear_stress_var
  []
[]

[Materials]
  [stress]
    type = ComputeLinearElasticStress
  []
  [elastic]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 0.3
    shear_modulus = 100
  []
  []

[Executioner]
  # type = Steady
  type = Transient
  num_steps = 1
  solve_type = PJFNK
  petsc_options_iname = '-pctype'
  petsc_options_value = 'lu'
  nl_max_its = 150
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  perf_graph = true
[]
