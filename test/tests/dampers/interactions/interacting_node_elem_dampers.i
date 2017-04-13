# This model tests interactions between nodal and element dampers.
# The test verifies that the minimum of the value of a nodal and
# element damper is always used.
# If run with the nodal1 and elem1 dampers active, the element damper
# will govern.  With nodal2 and elem2 dampers, the nodal damper governs.

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
  active = 'nodal1 elem1'
  [./nodal1]
    #gives a damping of 0.3333 on step 6
    type = BoundingValueNodalDamper
    min_value = 0.0
    max_value = 1.0
    variable = u
  [../]
  [./elem1]
    #gives a damping of 0.141536 on step 6
    type = BoundingValueElementDamper
    min_value = 0.0
    max_value = 1.012
    variable = u
  [../]
  [./nodal2]
    #gives a damping of 0.3333 on step 6
    type = BoundingValueNodalDamper
    min_value = 0.0
    max_value = 1.0
    variable = u
  [../]
  [./elem2]
    #gives a damping of 0.743318 on step 6
    type = BoundingValueElementDamper
    min_value = 0.0
    max_value = 1.02
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
