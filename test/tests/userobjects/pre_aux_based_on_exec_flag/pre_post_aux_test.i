[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  nx = 2
  ymin = 0
  ymax = 1
  ny = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  [../]
[]

[AuxVariables]
 [w1]
   order = FIRST
   family = LAGRANGE
   initial_condition = 2
 []
 [w2]
   order = FIRST
   family = LAGRANGE
 []
 [w3]
   order = FIRST
   family = LAGRANGE
 []
 [w4]
   order = FIRST
   family = LAGRANGE
 []
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  # The purpose of this auxkernel is to provide the variable w1
  # and the scalepostprocessors included below will either get
  # an updated w1 or the previous w1 value depending on whether
  # they are forced in preaux or postaux
  [NormalizationAuxW1]
    type = NormalizationAux
    variable = w1
    source_variable = u
    shift = -100.0
    normalization = 1.0
    execute_on = 'INITIAL FINAL'
  []
  # This establishes a dependency for scale_initial on exec INITIAL
  [NormalizationAuxINITIAL]
    type = NormalizationAux
    variable = w2
    source_variable = u
    normalization = scale_initial
    execute_on = 'INITIAL'
  []
  # This establishes a dependency for scale_initial on exec TIMESTEP_END
  [NormalizationAuxTIMESTEP_END]
    type = NormalizationAux
    variable = w3
    source_variable = u
    normalization = scale_td_end
    execute_on = 'TIMESTEP_END'
  []
  # This establishes a dependency for scale_initial on exec FINAL
  [NormalizationAuxFINAL]
    type = NormalizationAux
    variable = w4
    source_variable = u
    normalization = scale_final
    execute_on = 'FINAL'
  []
[]

[Postprocessors]
  #
  # scalePostAux always gets run post_aux
  #
  [./total_u1]
    type = ElementIntegralVariablePostprocessor
    variable = w1
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END FINAL'
  [../]
  [./scalePostAux]
    type = ScalePostprocessor
    value = total_u1
    scaling_factor = 1
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END FINAL'
  [../]
  #
  # shoule only run pre_aux on initial
  #
  [./total_u2]
    type = ElementIntegralVariablePostprocessor
    variable = w1
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END FINAL'
  [../]
  [./scale_initial]
    type = ScalePostprocessor
    value = total_u2
    scaling_factor = 1
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END FINAL'
  [../]
  #
  # shoule be forced into preaux on timestep_end
  #
  [./total_u3]
    type = ElementIntegralVariablePostprocessor
    variable = w1
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END FINAL'
  [../]
  [./scale_td_end]
    type = ScalePostprocessor
    value = total_u3
    scaling_factor = 1
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END FINAL'
  [../]
  #
  # shoule be forced into preaux on final
  #
  [./total_u4]
    type = ElementIntegralVariablePostprocessor
    variable = w1
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END FINAL'
  [../]
  [./scale_final]
    type = ScalePostprocessor
    value = total_u4
    scaling_factor = 1
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END FINAL'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.0
  end_time = 2.0
[]

[Outputs]
  [console]
    type = Console
    execute_on = 'INITIAL FINAL'
  []
  [out]
    type = CSV
    execute_on = 'INITIAL FINAL'
  []
[]
