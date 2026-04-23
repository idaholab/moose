!include base.i

[Postprocessors]
  [test]
    type = TestGravityVectorInterface
    gravity_direction = '0 0 -1'
    gravity_vector = '0 0 -9.8'
    test_value = magnitude
  []
[]
