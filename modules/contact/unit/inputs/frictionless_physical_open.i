!include frictionless_physical.i

[GlobalParams]
  scaling := 1
[]

[BCs]
  [topz]
    function := '${starting_point} * t / 0.125 + ${offset}'
  []
[]
