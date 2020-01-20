# Plastic deformation, tensile failure, inclined normal = (0, 1, 0)
# With Young = 10, poisson=0.25 (Lame lambda=4, mu=4)
# applying the following
# deformation to the ymax surface of a unit cube:
# disp_x = 4*t
# disp_y = t
# disp_z = 3*t
# should yield trial stress:
# stress_yy = 12*t
# stress_yx = 16*t
# stress_yz = 12*t
# Use tensile strength = 6, we should return to stress_yy = 6,
# and stress_xx = stress_zz = 2*t up to t=1 when the system is completely
# plastic, so these stress components will not change
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]


[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_xy stress_xz stress_yy stress_yz stress_zz plastic_strain_xx plastic_strain_xy plastic_strain_xz plastic_strain_yy plastic_strain_yz plastic_strain_zz strain_xx strain_xy strain_xz strain_yy strain_yz strain_zz'
  [../]
[]

[BCs]
  [./bottomx]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./bottomz]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]

  [./topx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top
    function = 4*t
  [../]
  [./topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = t
  [../]
  [./topz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = top
    function = 3*t
  [../]
[]

[AuxVariables]
  [./f_shear]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f_tensile]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f_compressive]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl_shear]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl_tensile]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./ls]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./f_shear]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = f_shear
  [../]
  [./f_tensile]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 1
    variable = f_tensile
  [../]
  [./f_compressive]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 2
    variable = f_compressive
  [../]
  [./intnl_shear]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 0
    variable = intnl_shear
  [../]
  [./intnl_tensile]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 1
    variable = intnl_tensile
  [../]
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
  [./ls]
    type = MaterialRealAux
    property = plastic_linesearch_needed
    variable = ls
  [../]
[]

[Postprocessors]
  [./stress_xx]
    type = PointValue
    point = '0 0 0'
    variable = stress_xx
  [../]
  [./stress_xy]
    type = PointValue
    point = '0 0 0'
    variable = stress_xy
  [../]
  [./stress_xz]
    type = PointValue
    point = '0 0 0'
    variable = stress_xz
  [../]
  [./stress_yy]
    type = PointValue
    point = '0 0 0'
    variable = stress_yy
  [../]
  [./stress_yz]
    type = PointValue
    point = '0 0 0'
    variable = stress_yz
  [../]
  [./stress_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  [../]
  [./strainp_xx]
    type = PointValue
    point = '0 0 0'
    variable = plastic_strain_xx
  [../]
  [./strainp_xy]
    type = PointValue
    point = '0 0 0'
    variable = plastic_strain_xy
  [../]
  [./strainp_xz]
    type = PointValue
    point = '0 0 0'
    variable = plastic_strain_xz
  [../]
  [./strainp_yy]
    type = PointValue
    point = '0 0 0'
    variable = plastic_strain_yy
  [../]
  [./strainp_yz]
    type = PointValue
    point = '0 0 0'
    variable = plastic_strain_yz
  [../]
  [./strainp_zz]
    type = PointValue
    point = '0 0 0'
    variable = plastic_strain_zz
  [../]
  [./straint_xx]
    type = PointValue
    point = '0 0 0'
    variable = strain_xx
  [../]
  [./straint_xy]
    type = PointValue
    point = '0 0 0'
    variable = strain_xy
  [../]
  [./straint_xz]
    type = PointValue
    point = '0 0 0'
    variable = strain_xz
  [../]
  [./straint_yy]
    type = PointValue
    point = '0 0 0'
    variable = strain_yy
  [../]
  [./straint_yz]
    type = PointValue
    point = '0 0 0'
    variable = strain_yz
  [../]
  [./straint_zz]
    type = PointValue
    point = '0 0 0'
    variable = strain_zz
  [../]
  [./f_shear]
    type = PointValue
    point = '0 0 0'
    variable = f_shear
  [../]
  [./f_tensile]
    type = PointValue
    point = '0 0 0'
    variable = f_tensile
  [../]
  [./f_compressive]
    type = PointValue
    point = '0 0 0'
    variable = f_compressive
  [../]
  [./intnl_shear]
    type = PointValue
    point = '0 0 0'
    variable = intnl_shear
  [../]
  [./intnl_tensile]
    type = PointValue
    point = '0 0 0'
    variable = intnl_tensile
  [../]
  [./iter]
    type = PointValue
    point = '0 0 0'
    variable = iter
  [../]
  [./ls]
    type = PointValue
    point = '0 0 0'
    variable = ls
  [../]
[]


[UserObjects]
  [./coh]
    type = TensorMechanicsHardeningConstant
    value = 80
  [../]
  [./tanphi]
    type = TensorMechanicsHardeningConstant
    value = 0.5
  [../]
  [./tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 0.1111077
  [../]
  [./t_strength]
    type = TensorMechanicsHardeningConstant
    value = 6
  [../]
  [./c_strength]
    type = TensorMechanicsHardeningConstant
    value = 40
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '4 4'
  [../]
  [./admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = stress
    perform_finite_strain_rotations = false
  [../]
  [./stress]
    type = CappedWeakInclinedPlaneStressUpdate
    normal_vector = '0 1 0'
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    tip_smoother = 0
    smoothing_tol = 0
    yield_function_tol = 1E-5
  [../]
[]

[Executioner]
  end_time = 2
  dt = 1
  type = Transient
[]

[Outputs]
  file_base = small_deform_inclined3
  csv = true
[]

