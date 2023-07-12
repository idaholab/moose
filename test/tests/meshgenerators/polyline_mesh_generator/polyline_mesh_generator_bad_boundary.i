[Mesh]
  [poly]
    type = PolyLineMeshGenerator
    points = '-1.0 0.0 0.0
              1.0 1.0 1.0
              1.0 2.0 3.0'
    # This should error for 16-bit boundary id configs:
    start_boundary = 40000
    # This should error even for 64-bit boundary id configs:
    end_boundary = 123456789012345678901234567890
  []
[]
