# This test provides an example of an individual LPS viscoplasticity model

!include lps_single.i
dt := 0.001

[Executioner]
  end_time := 0.5
[]

[Outputs]
  execute_on = 'FINAL'
[]
