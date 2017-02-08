# This test checks that the usage of an old/older (stateful) material property
# does not create a dependency on that property for the purposes of
# dependency resolution for material property evaluation.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 2
  [../]
[]

[Materials]
  [./mat1]
    type = CoupledMaterial
    mat_prop = 'prop-a'
    coupled_mat_prop = 'prop-b'
    use_old_prop = true
    block = 0
  [../]

  [./mat2]
    type = CoupledMaterial
    mat_prop = 'prop-b'
    coupled_mat_prop = 'prop-a'
    use_old_prop = false
    block = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Debug]
  show_material_props = true
[]
