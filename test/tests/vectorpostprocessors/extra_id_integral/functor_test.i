[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    subdomain_ids = '0 0 0 0 0
                     0 0 0 0 0
                     1 1 1 1 1
                     1 1 1 1 1
                     1 1 1 1 1'
  []
  [add_id]
    type = ParsedExtraElementIDGenerator
    input = gmg
    extra_elem_integer_name = id
    expression = 'if(x<0.2,1,2)'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [bf]
    type = BodyForce
    variable = u
    value = 1
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[VectorPostprocessors]
  [eeid]
    type = ExtraIDIntegralVectorPostprocessor
    id_name = id
    variable = u
    force_preaux = true
    #spatial_value_name = u
  []
[]

[AuxVariables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = FunctorAux
      functor = eeid
      execute_on = timestep_end
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
