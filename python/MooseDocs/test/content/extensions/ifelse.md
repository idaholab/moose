# IfElse Extension

!if function=hasMooseApp('MooseTestApp')
if shown

!if function=hasMooseApp('RayTracingApp')
skip

!else
else shown

!if function=hasMooseApp('RayTracingApp')
skip

!elif function=hasMooseApp('MooseTestApp')
elif shown

!else
skip

!if function=hasMooseApp('RayTracingApp')
skip

!elif function=hasMooseApp('RayTracingApp')
skip

!else
else shown with elif

!if! function=hasMooseApp('MooseTestApp')
block if shown
!if-end!

!elif function=hasMooseApp('MooseTestApp')
not shown

!else
not shown
