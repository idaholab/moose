# Two HEX8 elements, fixed at the bottom, with the top face rotated by 0.6 rad about z and
# pulled by 30% of the height in one step. det(F) varies within each element, so F-bar changes
# the solution. The total and updated Lagrangian formulations must give the same converged
# displacements and Cauchy stress, so the tests for both compare against one gold file.
theta = 0.6

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nz = 2
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = FINITE
        add_variables = true
        new_system = true
        formulation = UPDATED
        volumetric_locking_correction = true
      []
    []
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = back
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = back
    value = 0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [rotate_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = front
    function = 'x*cos(${theta}*t) - y*sin(${theta}*t) - x'
  []
  [rotate_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = front
    function = 'x*sin(${theta}*t) + y*cos(${theta}*t) - y'
  []
  [pull_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = '0.3*t'
  []
[]

[Materials]
  active = 'elastic_tensor flow_stress stress s_zz'
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 6.88e4
    poissons_ratio = 0.25
  []
  [flow_stress]
    type = DerivativeParsedMaterial
    property_name = flow_stress
    expression = '320+688*effective_plastic_strain'
    material_property_names = 'effective_plastic_strain'
    additional_derivative_symbols = 'effective_plastic_strain'
    derivative_order = 2
    compute = false
  []
  [stress]
    type = ComputeSimoHughesJ2PlasticityStress
    flow_stress_material = flow_stress
  []
  [neohookean]
    type = ComputeNeoHookeanStress
    lambda = 2.75e4
    mu = 2.75e4
  []
  [s_zz]
    type = RankTwoCartesianComponent
    rank_two_tensor = cauchy_stress
    property_name = s_zz
    index_i = 2
    index_j = 2
  []
[]

[Postprocessors]
  # A node at mid-height on the free side
  [disp_x]
    type = PointValue
    variable = disp_x
    point = '0.5 0.5 0.5'
  []
  [disp_y]
    type = PointValue
    variable = disp_y
    point = '0.5 0.5 0.5'
  []
  [s_zz]
    type = ElementAverageMaterialProperty
    mat_prop = s_zz
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = none
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-10
  dt = 1
  end_time = 1
[]

[Outputs]
  csv = true
[]
