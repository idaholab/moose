# This model tests the BoundingValueNodalDamper. The converged solution
# for u starts out in the range from 0 to 1, but after several steps,
# a volumetric source drives it to a value greater than 1, which is
# outside the range of the damper. At that point, the solution can
# no longer converge, and the model errors out with a failure to converge.
# The test verifies that the damper computes the correct value in the first
# nonlinear iteration when the solution exceeds the bounds.

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Variables]
  [./u]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./source]
    type = BodyForce
    variable = u
    function = 't'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Dampers]
  [./bounding_value_damp]
    type = BoundingValueNodalDamper
    min_value = 0.0
    max_value = 1.0
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  end_time = 3.0
  dt = 0.5
  dtmin = 0.5
  nl_max_its = 5
[]
