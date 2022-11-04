# The left subdomain has block id 0
# The right subdomain has block id 1
#
# newton_0 is a material defined on block 0
# newton_1 is a material defined on block 1
#
# We expect no cyclic dependency between newton_0 and newton_!

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 1
  []
  [right]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    block_id = 1
  []
[]

[Problem]
  solve = false
[]

[Materials]
  [recompute_props_0]
    type = RecomputeMaterial
    block = 0
    f_name = 'f'
    f_prime_name = 'f_prime'
    p_name = 'p'
    compute = false # make this material "discrete"
  []
  [newton_0]
    type = NewtonMaterial
    block = 0
    f_name = 'f'
    f_prime_name = 'f_prime'
    p_name = 'p'
    material = 'recompute_props_0'
  []
  [recompute_props_1]
    type = RecomputeMaterial
    block = 1
    f_name = 'f'
    f_prime_name = 'f_prime'
    p_name = 'p'
    compute = false # make this material "discrete"
  []
  [newton_1]
    type = NewtonMaterial
    block = 1
    f_name = 'f'
    f_prime_name = 'f_prime'
    p_name = 'p'
    material = 'recompute_props_1'
  []
[]

[Executioner]
  type = Steady
[]
