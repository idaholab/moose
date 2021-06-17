# ActivateElementsByPath

This user object uses the +function path+ as the metric to activate (add) an element by moving the element from an "inactive" subdomain to the "active" subdomain. It uses the user provided points $(x(t), y(t), z(t))$ with components defined by the functions specified by the parameters `function_x`, `function_y`, and `function_z` in the input. An element is activated at time $t_0$ if this element is close (distance < `activate_distance`) to the point $(x(t_0), y(t_0), z(t_0))$.

!syntax parameters /UserObjects/ActivateElementsByPath

!syntax inputs /UserObjects/ActivateElementsByPath

!syntax children /UserObjects/ActivateElementsByPath
