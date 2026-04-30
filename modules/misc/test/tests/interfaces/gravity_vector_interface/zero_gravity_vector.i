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
  [dir_x]
    type = TestGravityVectorInterface
    test_value = dir_x
  []
  [dir_y]
    type = TestGravityVectorInterface
    test_value = dir_y
  []
  [dir_z]
    type = TestGravityVectorInterface
    test_value = dir_z
  []
[]

[Outputs]
  csv = true
[]
