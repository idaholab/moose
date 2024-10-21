#
# Cavity Pressure Test
#
# This test is designed to compute a negative number of moles
# to trigger an error check in the CavityPressureUserObject.
# The negative number of moles is achieved by supplying an
# open volume to the InternalVolume postprocessor, which
# calculates a negative volume.

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 2
[]

[Functions]
  [temperature]
    type = PiecewiseLinear
    x = '0 1'
    y = '1 2'
    scale_factor = 100
  []
[]

[Variables]
  [temperature]
    initial_condition = 100
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [block]
    strain = FINITE
    add_variables = true
    use_automatic_differentiation = true
  []
[]

[Kernels]
  [heat]
    type = Diffusion
    variable = temperature
    use_displaced_mesh = true
  []
[]

[BCs]
  [no_x]
    type = ADDirichletBC
    variable = disp_r
    boundary = left
    value = 0.0
  []
  [no_y]
    type = ADDirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
  [temperatureInterior]
    type = ADFunctionDirichletBC
    boundary = 2
    function = temperature
    variable = temperature
  []
  [CavityPressure]
    [pressure]
      boundary = 'top bottom right'
      initial_pressure = 10e5
      R = 8.3143
      output_initial_moles = initial_moles
      temperature = aveTempInterior
      volume = internalVolume
      startup_time = 0.5
      output = ppress
      use_automatic_differentiation = true
    []
  []
[]

[Materials]
  [elastic_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stress1]
    type = ADComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm       lu'

  nl_abs_tol = 1e-10
  l_max_its = 20
  dt = 0.5
  end_time = 1.0
  use_pre_SMO_residual = true
[]

[Postprocessors]
  [internalVolume]
    type = InternalVolume
    boundary = 'top bottom right'
    execute_on = 'initial linear'
  []
  [aveTempInterior]
    type = AxisymmetricCenterlineAverageValue
    boundary = left
    variable = temperature
    execute_on = 'initial linear'
  []
[]

[Outputs]
  exodus = false
[]
