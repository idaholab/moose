# Exception: incorrect userobject types
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


[Variables]
  [./x_disp]
  [../]
  [./y_disp]
  [../]
  [./z_disp]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'x_disp y_disp z_disp'
  [../]
[]


[UserObjects]
  [./coh]
    type = TensorMechanicsHardeningConstant
    value = -1
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
    value = 1
  [../]
  [./c_strength]
    type = TensorMechanicsHardeningConstant
    value = 2
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 1E6'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    displacements = 'x_disp y_disp z_disp'
  [../]
  [./stress]
    type = ComputeCappedWeakPlaneStress
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    tip_smoother = 0
    smoothing_tol = 1
    yield_function_tol = 1E-5
  [../]
[]

[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]
