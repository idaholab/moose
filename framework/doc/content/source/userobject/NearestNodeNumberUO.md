# NearestNodeNumberUO

This UserObject computes the closest node to a given point.  (If there is a tie, then the node with the smallest ID is used.)  It is used by the [GeochemistryConsoleOutput](http://mooseframework.org/docs/doxygen/modules/GeochemistryConsoleOutput.html) object to identify the node at which output is required.

!alert note
You will probably want to set execute_on to initial to avoid unnecessary computations, unless you have adaptive meshing, in which case set execute_on to initial and timestep_begin.

!syntax parameters /UserObjects/NearestNodeNumberUO

!syntax inputs /UserObjects/NearestNodeNumberUO

!syntax children /UserObjects/NearestNodeNumberUO
