[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./power]
    type = PReaction
    variable = u
    coefficient = 0.2
    power = -5
    # Comment out this will make fixed point iteration converged in one iteration.
    # However, this makes the solving diverge and require a proper initial condition (>1.00625).
    vector_tags = 'previous'
  [../]
[]

[BCs]
  [./left]
    type = VacuumBC
    variable = u
    boundary = left
  [../]

  [./right]
    type = NeumannBC
    variable = u
    boundary = right
    value = 10
  [../]
[]

[Postprocessors]
  [./unorm]
    type = ElementL2Norm
    variable = u
  [../]
[]

[Problem]
  type = FixedPointProblem
  fp_tag_name = 'previous'
[]

[Executioner]
  type = FixedPointSteady
  nl_rel_tol = 1e-50
  line_search = none
  n_max_nonlinear_pingpong = 2
[]
