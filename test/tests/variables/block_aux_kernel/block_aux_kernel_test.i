###########################################################
# This is a simple test of the AuxVariable System.
# A single discretized explicit variable is added to the
# system which is independent of the nonlinear variables
# being solved for by the solver.
#
# @Requirement F5.10
###########################################################


[Mesh]
  file = gap_test.e
  # This test uses the geometric search system, which does not currently work
  # in parallel with DistributedMesh enabled.  For more information, see #2121.
  parallel_type = replicated
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./distance]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff u_time'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./u_time]
    type = TimeDerivative
    variable = u
  [../]
[]

[AuxKernels]
  [./x]
    type = FunctionAux
    variable = disp_x
    function = 0
    block = 1
  [../]

  [./y]
    type = FunctionAux
    variable = disp_y
    function = 0
    block = 1
  [../]

  [./z]
    type = FunctionAux
    variable = disp_z
    function = t
    block = 1
  [../]
  [./gap_distance]
    type = NearestNodeDistanceAux
    variable = distance
    boundary = 2
    paired_boundary = 3
  [../]

  [./gap_distance2]
    type = NearestNodeDistanceAux
    variable = distance
    boundary = 3
    paired_boundary = 2
  [../]
[]

[BCs]
  active = 'block1_left block1_right block2_left block2_right'

  [./block1_left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./block1_right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]

  [./block2_left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./block2_right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  dt = 1.0
  num_steps = 8
[]

[Outputs]
  file_base = out
  exodus = true
[]
