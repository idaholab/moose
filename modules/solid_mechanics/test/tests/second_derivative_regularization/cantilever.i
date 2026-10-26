reg_kernel = SecondDerivativeRegularization
regularization_type = huhu
coefficient = 10

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD9
  xmax = 10
  ymax = 1
  nx = 1
  ny = 1
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    planar_formulation = PLANE_STRAIN
    add_variables = true
  []
[]

[Kernels]
  [regularization_x]
    type = ${reg_kernel}
    variable = disp_x
    regularization = ${regularization_type}
    coefficient = ${coefficient}
  []
  [regularization_y]
    type = ${reg_kernel}
    variable = disp_y
    regularization = ${regularization_type}
    coefficient = ${coefficient}
  []
[]

[BCs]
  [clamp_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [clamp_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0
  []
  [tip_shear]
    type = NeumannBC
    variable = disp_y
    boundary = right
    value = -1e-3
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Postprocessors]
  [tip_disp_y]
    type = PointValue
    variable = disp_y
    point = '10 0.5 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
[]
