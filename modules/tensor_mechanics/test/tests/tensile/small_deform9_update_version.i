# A single unit element is stretched in a complicated way
# that the trial stress is
#
#      1.16226      -0.0116587       0.0587872
#     -0.0116587         1.12695       0.0779428
#      0.0587872       0.0779428        0.710169
#
# This has eigenvalues
# la = {0.68849, 1.14101, 1.16987}
# and eigenvectors
#
# {-0.125484, -0.176871, 0.976202}
# {-0.0343704, -0.982614, -0.182451}
# {0.9915, -0.0564471, 0.117223}
#
# The tensile strength is 0.5 and Young=1 and Poisson=0.25.
# Using smoothing_tol=0.01, the return-map algorithm should
# return to, approximately, stress_I=stress_II=0.5.  This
# is a reduction of 0.65, so stress_III is approximately
# 0.68849 - 0.25 * 0.65 * 2 = 0.36.  The stress_I reduction of
# 0.67 gives an internal parameter of
# 0.67 / (E(1-v)/(1+v)/(1-2v)) = 0.558
# The final stress is
#
# {0.498, -0.003, 0.017},
# {-0.003, 0.495, 0.024},
# {0.017, 0.024,  0.367}

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
    strain = finite
    generate_output = 'max_principal_stress mid_principal_stress min_principal_stress stress_xx stress_xy stress_xz stress_yy stress_yz stress_zz'
  [../]
[]

[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '3*x+2*y+z'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '3*x-4*y'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = 'x-2*z'
  [../]
[]

[AuxVariables]
  [./f0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./f0_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = f0
  [../]
  [./f1_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 1
    variable = f1
  [../]
  [./f2_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 2
    variable = f2
  [../]
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
  [./intnl_auxk]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 0
    variable = intnl
  [../]
[]

[Postprocessors]
  [./s_I]
    type = PointValue
    point = '0 0 0'
    variable = max_principal_stress
  [../]
  [./s_II]
    type = PointValue
    point = '0 0 0'
    variable = mid_principal_stress
  [../]
  [./s_III]
    type = PointValue
    point = '0 0 0'
    variable = min_principal_stress
  [../]
  [./s_xx]
    type = PointValue
    point = '0 0 0'
    variable = stress_xx
  [../]
  [./s_xy]
    type = PointValue
    point = '0 0 0'
    variable = stress_xy
  [../]
  [./s_xz]
    type = PointValue
    point = '0 0 0'
    variable = stress_xz
  [../]
  [./s_yy]
    type = PointValue
    point = '0 0 0'
    variable = stress_yy
  [../]
  [./s_yz]
    type = PointValue
    point = '0 0 0'
    variable = stress_yz
  [../]
  [./s_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  [../]
  [./f0]
    type = PointValue
    point = '0 0 0'
    variable = f0
  [../]
  [./f1]
    type = PointValue
    point = '0 0 0'
    variable = f1
  [../]
  [./f2]
    type = PointValue
    point = '0 0 0'
    variable = f2
  [../]
  [./iter]
    type = PointValue
    point = '0 0 0'
    variable = iter
  [../]
  [./intnl]
    type = PointValue
    point = '0 0 0'
    variable = intnl
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 0.5
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.25
  [../]
  [./tensile]
    type = TensileStressUpdate
    tensile_strength = ts
    smoothing_tol = 0.001
    yield_function_tol = 1.0E-12
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = tensile
    perform_finite_strain_rotations = false
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform9_update_version
  exodus = false
  [./csv]
    type = CSV
  [../]
[]
