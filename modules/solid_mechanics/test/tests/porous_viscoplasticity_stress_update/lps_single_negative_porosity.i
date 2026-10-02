# This test provides an example of an individual LPS viscoplasticity model

!include lps_single.i

porosity_name := negative_porosity

[Materials]
  [porosity_negative]
    type = ADParsedMaterial
    property_name = negative_porosity
    expression = -0.1
    outputs = all
  []
  [lps]
    negative_behavior= ZERO
  []
[]
