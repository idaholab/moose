###########################################################
# Demonstrating ExplicitMixedOrderStasis
#
# Also demonstrating:
#  - correctly setting ICs in explicit systems
#  - correctly setting time step via a Postprocessor in explicit systems
#  - correctly setting the in_stasis Postprocessor
# It is easy to mix up the correct setting of these by one time step!
#
# A single element of length 1, Young modulus 1 and density = 10
# has its left side fixed at displacement = 0.5 .
# The right-side equilibrium position is clearly displacement = 0.5 .
# The right-side displacement relative to its equilibrium position, u(t),
# is recorded.  u(t) = disp_x - 0.5
# Small strain and linear elastic stress is used.
# The dynamics is M\ddot{u} + ku = 0
# where M = 5, k = 1, and u(t=0) = -0.5, and \dot{u}(t=0) = 0
#
# For t < 4.5:
#  - in_stasis = 1 and dt = 1
#  - therefore, expect u = -0.5 for all these time steps
# For 4.5 <= t < 6.5
#  - in_stasis = 0 and dt = 1
#  - Time = 5,
#       a = -(-0.5)/5 = 0.1
#       v = 0 + 0.1 * 1 = 0.1
#       u(5) = -0.5 + 0.1 * 1 = -0.4
#  - Time = 6,
#       a = -(-0.4)/5 = 0.08
#       v = 0.1 + 0.08 * 1 = 0.18
#       u(6) = -0.4 + 0.18 * 1 = -0.22
# For 6.5 <= t < 10.5
#  - in_stasis = 1 and dt = 4
#  - Time = 10
#       a = 0
#       v = 0
#       u(10) = -0.22
# For 10.5 <= t < 14.5
#  - in_stasis = 0 and dt = 2
#  - Time = 12, note at dt_old=1 will be used in the central difference
#       a = -(-0.22)/5 = 0.044
#       v = 0 + 0.044 * (2 + 1)/2 = 0.066
#       u(12) = -0.22 + 0.066 * 2 = -0.088
#  - Time = 14
#       a = -(-0.088)/5 = 0.0176
#       v = 0.066 + 0.0176 * 2 = 0.1012
#       u(12) = -0.088 + 0.1012 * 2 = 0.1144
# For 14.5 <= t < 18.5
#  - in_stasis = 1 and dt = 1
#  - Time = 18
#       a = 0
#       v = 0
#       u(18) = 0.1144
# For 18.5 <= t < 20.5
#  - in_stasis = 0 and dt = 1
#  - Time = 19, note at dt_old=2 will be used in the central difference
#       a = -0.1144/5 = -0.02288
#       v = 0 - 0.02288 * (1 + 2)/2 = -0.03432
#       u(19) = 0.1144 - 0.03432 * 1 = 0.08008
#  - Time = 20
#       a = -(0.08008)/5 = -0.016016
#       v = -0.03432 - 0.016016 * 1 = -0.050336
#       u(20) = 0.08008 - 0.050336 * 1 = 0.029744
###########################################################

[GlobalParams]
  displacements = disp_x
[]

[Problem]
  extra_tag_matrices = mass
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [disp_x]
  []
[]

[Kernels]
  [TM_DynamicSolidMechanics0]
    type = DynamicStressDivergenceTensors
    alpha = 0
    component = 0
    use_displaced_mesh = false
    variable = disp_x
    zeta = 0
    implicit = false
  []
  [massmatrix]
    type = MassMatrix
    density = density
    matrix_tags = mass
    variable = disp_x
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0
    implicit = false
  []
  [strain_block]
    type = ComputeSmallStrain
    displacements = disp_x
    implicit = false
  []
  [stress_block]
    type = ComputeLinearElasticStress
    implicit = false
  []
  [density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 10
    implicit = false
  []
[]

[BCs]
  [left]
    type = ExplicitDirichletBC
    variable = disp_x
    boundary = left
    value = 0.5
    implicit = false
  []
[]

[ICs]
# this is necessary, because explicit time integration calculates the
# strain and stress based on old values.  Hence, on the first time step
# these will be zero, unless we set the old values of disp_x to the
# values of the BC.  If this IC is not used, then the simulation will
# only "get going" at the second time step, when the BC has been active
# for one time step so the strain and stress are non zero.
  [u_old_left]
    type = ConstantIC
    variable = disp_x
    boundary = left
    state = OLD
    value = 0.5
  []
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = ExplicitMixedOrderStasis
    mass_matrix_tag = mass
    second_order_vars = disp_x
    in_stasis = in_stasis
  []
  [TimeStepper]
    type = PostprocessorDT
    postprocessor = next_dt
    dt = 1
  []
  start_time = 0.0
  end_time = 20
[]

[Postprocessors]
  [in_stasis]
    type = ParsedPostprocessor
    expression = 'if((t < 4.5) | ((t > 6.5) & (t < 10.5)) | ((t > 14.5) & (t < 18.5)), 1, 0)'
# Note: need to execute_on = TIMESTEP_BEGIN
# The execute_on = INITIAL is because this unusual case starts in stasis
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    use_t = true
  []
  [next_dt]
    type = ParsedPostprocessor
    use_t = true
    expression = 'if(t < 5.5, 1, if(t < 9.5, 4, if(t < 13.5, 2, 1)))'
  []
  [disp_x]
    type = AverageNodalVariableValue
    variable = disp_x
    boundary = right
    outputs = 'none'
  []
  [u]
    type = ParsedPostprocessor
    pp_names = disp_x
    pp_symbols = disp_x
    expression = 'disp_x - 0.5'
  []
[]

[Outputs]
  csv = true
[]
