# Test designed to compare results and active time between SH/LinearStrainHardening
# material vs TM j2 plastic user object. As number of elements increases, TM
# active time increases at a much higher rate than SM. Testing at 4x4x4
# (64 elements).
#
# plot vm_stress vs intnl to see constant hardening
#
# Original test located at:
# tensor_mechanics/tests/j2_plasticity/hard1.i

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 4
  ny = 4
  nz = 4
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_zz vonmises_stress effective_plastic_strain'
  []
[]

[AuxVariables]
  [intnl]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [intnl]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_internal_parameter
    variable = intnl
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [back]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
  [z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = 't/60'
  []
[]

[UserObjects]
  [str]
    type = TensorMechanicsHardeningConstant
    value = 2.4e2
  []
  [j2]
    type = TensorMechanicsPlasticJ2
    yield_strength = str
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-9
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    #with E = 2.1e5 and nu = 0.3
    #Hooke's law: E-nu to Lambda-G
    C_ijkl = '121154 80769.2'
  []
  [mc]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1E-9
    plastic_models = j2
    tangent_operator = elastic
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-10
  l_tol = 1e-4
  start_time = 0.0
  end_time = 0.5
  dt = 0.01
[]

[Postprocessors]
  [stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [intnl]
    type = ElementAverageValue
    variable = intnl
  []
  [eq_pl_strain]
    type = PointValue
    point = '0 0 0'
    variable = effective_plastic_strain
  []
  [vm_stress]
    type = PointValue
    point = '0 0 0'
    variable = vonmises_stress
  []
[]

[Outputs]
  csv = true
  print_linear_residuals = false
  perf_graph = true
[]
