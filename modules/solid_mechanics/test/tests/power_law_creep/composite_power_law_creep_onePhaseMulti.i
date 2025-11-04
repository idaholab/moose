# 1x1x1 unit cube with uniform pressure on top face and 2 phases with different materials

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 6
  zmax = 1
  xmax = 1
  ymax = 1
[]

[Variables]
  [temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1000.0
  []
[]

[ICs]
  [phase1IC]
    type = BoundingBoxIC
    x1 = -1
    x2 = 1.5
    y1 = -1
    y2 = 1.5
    z1 = -1
    z2 = 1.0
    inside = 1
    outside = 0
    variable = phase1
    int_width=0.01
  []
  [phase2IC]
    type = BoundingBoxIC
    x1 = -1
    x2 = 1.5
    y1 = -1
    y2 = 1.5
    z1 = -1
    z2 = 1.0
    inside = 0
    outside = 1
    variable = phase2
    int_width=0.01
  []
[]

[AuxVariables]
  [phase1]
  []
  [phase2]
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    incremental = true
    add_variables = true
    generate_output = 'stress_yy creep_strain_xx creep_strain_yy creep_strain_zz elastic_strain_yy'
  []
[]

[Functions]
  [top_pull]
    type = PiecewiseLinear
    x = '0 1'
    y = '1 1'
  []
[]

[Kernels]
  [heat]
    type = Diffusion
    variable = temp
  []
  [heat_ie]
    type = TimeDerivative
    variable = temp
  []
[]

[BCs]
  [u_top_pull]
    type = Pressure
    variable = disp_y
    boundary = top
    factor = -10.0e6
    function = top_pull
  []
  [u_bottom_fix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [u_yz_fix]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [u_xy_fix]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
  [temp_fix]
    type = DirichletBC
    variable = temp
    boundary = 'bottom top'
    value = 1000.0
  []
[]

[Materials]
  [elasticity_tensor1]
    type = ComputeIsotropicElasticityTensor
    base_name = C1
    youngs_modulus = 2e11
    poissons_ratio = 0.3
  []
  [elasticity_tensor2]
    type = ComputeIsotropicElasticityTensor
    base_name = C2
    youngs_modulus = 2e11
    poissons_ratio = 0.3
  []
  [h1]
    type = ParsedMaterial
    property_name = h1
    coupled_variables = phase1
    expression = '0.5*tanh(20*(phase1-0.5))+0.5'
  []
  [h2]
    type = ParsedMaterial
    property_name = h2
    coupled_variables = phase2
    expression = '0.5*tanh(20*(phase2-0.5))+0.5'
  []
  [./C]
    type = CompositeElasticityTensor
    coupled_variables = 'phase1 phase2'
    tensors = 'C1   C2'
    weights = 'h1   h2'
  [../]
  [radial_return_stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'power_law_creep plas'
    tangent_operator = elastic
  []
  [power_law_creep]
    type = CompositePowerLawCreepStressUpdate
    coefficient = '1e-15 5e-20'
    n_exponent = '4      5'
    activation_energy = '3.0e5 3.5e5'
    switching_functions = 'h1  h1'
    temperature = temp
  []
  [./plas]
    type = IsotropicPlasticityStressUpdate
    hardening_constant = 1
    yield_stress = 1e30
  [../]
[]

[VectorPostprocessors]
  [./soln]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    sort_by = x
    variable = 'disp_x disp_y disp_z creep_strain_xx creep_strain_yy creep_strain_zz'
    start_point = '0 0 0.0'
    end_point = '1.0 1.0 1.0'
    num_points = 5
    outputs = tests
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 20
  nl_max_its = 20
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-10
  l_tol = 1e-10
  start_time = 0.0
  end_time = 1.0
  num_steps = 10
  dt = 0.1
[]

[Outputs]
  exodus = false
  [./tests]
    type = CSV
    execute_on = final
  [../]
[]

