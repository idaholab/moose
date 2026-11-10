# Functors as symbols in the displacement expressions. 'swelling' is a spatially
# varying functor material property referenced by its own name, 'alpha' is a
# constant functor referenced through the symbol 'a'. Functors are sampled at the
# node in its original position, so after two steps the coordinates are
# x + (0.1*x + 0.05) and y + 0.2*y, i.e. the same as after one step (an
# implementation sampling the already displaced node would instead drift).
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
[]

[Variables]
  [u]
    initial_condition = 1
  []
[]

[FunctorMaterials]
  [swell]
    type = ParsedFunctorMaterial
    property_name = swelling
    expression = '0.1*x + 0.05'
  []
  [thermal]
    type = GenericFunctorMaterial
    prop_names = 'alpha'
    prop_values = '0.2'
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 2
[]

[UserObjects]
  [move]
    type = MoveNodesByParsedExpressionModifier
    block = 0
    displacement_x = 'swelling'
    displacement_y = 'a*y'
    functor_names = 'swelling alpha'
    functor_symbols = 'swelling a'
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'FINAL'
  []
[]
