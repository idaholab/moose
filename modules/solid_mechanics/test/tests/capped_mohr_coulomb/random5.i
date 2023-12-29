# Using CappedMohrCoulomb
# Plasticity models:
# Tensile strength = 1.5
# Compressive strength = 3.0
# Cohesion = 1.0
# Friction angle = dilation angle = 20deg
#
# Young = 1, Poisson = 0.3
#
# A line of elements is perturbed randomly, and return to the yield surface at each quadpoint is checked

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 100
  ny = 12
  nz = 1
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 12
  zmin = 0
  zmax = 1
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]


[ICs]
  [./x]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_x
  [../]
  [./y]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_y
  [../]
  [./z]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_z
  [../]
[]

[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '0'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '0'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '0'
  [../]
[]

[AuxVariables]
  [./Smax]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./Smid]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./Smin]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./Smax]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = Smax
    scalar_type = MaxPrincipal
  [../]
  [./Smid]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = Smid
    scalar_type = MidPrincipal
  [../]
  [./Smin]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = Smin
    scalar_type = MinPrincipal
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 1.5
  [../]
  [./cs]
    type = TensorMechanicsHardeningConstant
    value = 3.0
  [../]
  [./coh]
    type = TensorMechanicsHardeningConstant
    value = 1.0
  [../]
  [./phi]
    type = TensorMechanicsHardeningConstant
    value = 20
    convert_to_radians = true
  [../]
  [./psi]
    type = TensorMechanicsHardeningConstant
    value = 3
    convert_to_radians = true
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./capped_mc]
    type = CappedMohrCoulombStressUpdate
    tensile_strength = ts
    compressive_strength = cs
    cohesion = coh
    friction_angle = phi
    dilation_angle = psi
    smoothing_tol = 0.2
    yield_function_tol = 1.0E-12
    max_NR_iterations = 1000
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = capped_mc
    perform_finite_strain_rotations = false
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = random5
  exodus = true
[]
