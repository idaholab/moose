[Mesh]
  type = GeneratedMesh
  dim = 2
  uniform_refine = 4
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./ssm]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./conv]
    type = Convection
    variable = u
    velocity = '1 0 0'
  [../]
[]

[AuxKernels]
  [./ssm]
    type = MaterialRealAux
    variable = ssm
    property = diffusivity
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

[Materials]
  [./ssm]
    type = SpatialStatefulMaterial
    block = 0
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = -snes_mf_operator
  num_steps = 3
  dt = 1
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  marker = box
  [./Markers]
    [./box]
      type = BoxMarker
      bottom_left = '0.2 0.2 0'
      top_right = '0.4 0.4 0'
      inside = refine
      outside = coarsen
    [../]
  [../]
[]

[Output]
  exodus = true
[]

