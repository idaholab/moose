> Before running the model, consider the initial conditions and boundary conditions,
> and estimate what expected solution should be when the time-dependent term
> is included.

The initial condition is 300 K, and the boundary condition prescribed on the left-hand
side of the rectangular domain is also 300 K. A spatially uniform temperature
that increases linearly as a function of time from 300 K is prescribed on the right-hand
side. Because of the geometry of the domain and the boundary conditions, there should
be no variation in the solution in the $y$ direction.

The addition of the time-derivative term should cause a delayed response to the applied
temperature increase on the right-hand side of the domain. Instead of a linear temperature
increase from left to right, we would expect to initially see very little temperature
increase on the left side of the domain. The local temperature gradient should increase
as we go from the left to the right side of the domain.
