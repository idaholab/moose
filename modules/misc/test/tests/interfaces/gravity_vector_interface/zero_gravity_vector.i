!include base.i

[GlobalParams]
  gravity_vector = '0 0 0'
[]

[Postprocessors]
  [g_norm]
    type = TestGravityVectorInterface
    test_value = magnitude
  []
  [g_x]
    type = TestGravityVectorInterface
    test_value = x
  []
  [g_y]
    type = TestGravityVectorInterface
    test_value = y
  []
  [g_z]
    type = TestGravityVectorInterface
    test_value = z
  []
[]

[Outputs]
  csv = true
[]
