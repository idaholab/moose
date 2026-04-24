!include base.i

[Postprocessors]
  [test]
    type = TestGravityVectorInterface
    gravity_vector = '0 0 -9.8'
    gravity_magnitude = 9.8
    test_value = magnitude
  []
[]
