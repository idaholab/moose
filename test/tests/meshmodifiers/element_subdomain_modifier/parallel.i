[Problem]
  solve = false
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmax = 2
    ymax = 2
  []
  [block_1]
    type = SubdomainBoundingBoxGenerator
    input = 'gmg'
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '2 1 0'
  []
  [block_2]
    type = SubdomainBoundingBoxGenerator
    input = 'block_1'
    block_id = 2
    bottom_left = '0 1 0'
    top_right = '2 2 0'
  []
[]

[Variables]
  [dummy]
    block = 1
  []
[]

[AuxVariables]
  [u]
  []
[]

[AuxKernels]
  [cut]
    type = ParsedAux
    variable = 'u'
    expression = 'if (t>=1 & x<1 & y<1, 0, 1)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[MeshModifiers]
  [cut]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'u'
    criterion_type = 'BELOW'
    threshold = 0.99
    subdomain_id = 2
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  end_time = 2
  dt = 1
[]

[Outputs]
  exodus = true
[]
