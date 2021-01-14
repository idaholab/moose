[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1'
    dy = ' 1 1 1'
    subdomain_id = '1 2 3
                    4 5 6
                    7 8 9'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = L2_LAGRANGE
    initial_condition = 1
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [forcing]
    type = BodyForce
    variable = u
  []
[]

[DGKernels]
  [dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  []
  [test]
    type = MatDGKernel
    variable = u
    mat_prop = dummy
  []
[]

[BCs]
  [bc]
    type = PenaltyDirichletBC
    variable = u
    boundary = '0 1 2 3'
    penalty = 1e4
    value = 0
  []
[]

[Postprocessors]
  [block1_qps]
    type = NumElemQPs
    block = 1
  []
  [block5_qps]
    type = NumElemQPs
    block = 5
  []
  [block6_qps]
    type = NumElemQPs
    block = 6
  []
[]

[Materials]
  [dummy]
    type = GenericConstantMaterial
    block = '1 2 3 4 6 7 8 9'
    prop_names = dummy
    prop_values = 1
  []
  [qordermaterial]
    type = QuadratureMaterial
    block = 5
    property_name = dummy
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
