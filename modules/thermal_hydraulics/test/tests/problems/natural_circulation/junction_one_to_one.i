# This version uses JunctionOneToOne

!include base_params.i
!include base.i

[Components]
  [junction_heated_top]
    type = JunctionOneToOne1Phase
    connections = 'heated_pipe:out top_pipe:in'
  []
  [junction_top_cooled]
    type = JunctionOneToOne1Phase
    connections = 'top_pipe:out cooled_pipe:in'
  []
  [junction_cooled_bottom]
    type = JunctionOneToOne1Phase
    connections = 'cooled_pipe:out bottom_pipe:in'
  []
  [junction_bottom_heated]
    type = JunctionOneToOne1Phase
    connections = 'bottom_pipe:out heated_pipe:in'
  []
[]

