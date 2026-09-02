!include frictionless_physical.i

[BCs]
  [topz]
    function := '${starting_point} * t / 0.125 + ${offset}'
  []
[]
