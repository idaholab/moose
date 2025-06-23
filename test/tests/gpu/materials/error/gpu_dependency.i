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

[GPUKernels]
  [./diff]
    type = GPUCoefDiffusion
    variable = u
    coef = 0.1
  [../]
[]

[GPUBCs]
  [./left]
    type = GPUDirichletBC
    variable = u
    boundary = 'left'
    value = 1
  [../]

  [./right]
    type = GPUDirichletBC
    variable = u
    boundary = 'right'
    value = 2
  [../]
[]

[GPUMaterials]
  [./mat1]
    type = GPUCoupledMaterial
    mat_prop = 'prop-a'
    coupled_mat_prop = 'prop-b'
    use_old_prop = true
    block = 0
  [../]

  [./mat2]
    type = GPUCoupledMaterial
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
