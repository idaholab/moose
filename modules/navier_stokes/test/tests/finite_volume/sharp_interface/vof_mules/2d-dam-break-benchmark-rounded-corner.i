!include 2d-dam-break-benchmark.i

ic_corner_radius = 0.00142875

[Functions]
  [alpha_init]
    expression := 'if(x < ${dam_x}-${ic_corner_radius} & y < ${dam_y}, 1, if(x < ${dam_x} & y < ${dam_y}-${ic_corner_radius}, 1, if((x-(${dam_x}-${ic_corner_radius}))^2 + (y-(${dam_y}-${ic_corner_radius}))^2 < (${ic_corner_radius})^2, 1, 0)))'
  []
  [pressure_init]
    expression := '-(${rho_l}-${rho_g})*${g}*(${domain_dims_y}-${dam_y})*if(x < ${dam_x}-${ic_corner_radius} & y < ${dam_y}, 1, if(x < ${dam_x} & y < ${dam_y}-${ic_corner_radius}, 1, if((x-(${dam_x}-${ic_corner_radius}))^2 + (y-(${dam_y}-${ic_corner_radius}))^2 < (${ic_corner_radius})^2, 1, 0)))'
  []
[]
