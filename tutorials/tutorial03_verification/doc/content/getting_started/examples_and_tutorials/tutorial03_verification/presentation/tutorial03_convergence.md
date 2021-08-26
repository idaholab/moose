# Convergence

!---

## Theory

Numerical integration and [!ac](FEM) have +known rates of convergence to the exact solution+ for
decreasing time steps and element size.

!---

## Theory: Error

The error ($E$) shall be defined as the $L_2$-norm of the difference between the finite element
solution ($u_h$) and the known solution ($u$).

!equation
E = \sqrt{\sum_{i=0}^N (u^h_i - u_i)^2},

where $N$ is the number of nodes for the simulation.

This is just one common measure of error, many others would work just as well.

!---

## Theory: $\Delta t$

The time step size is easily computed as

!equation
\Delta t = t_i - t_{i-1},

where $t$ is time and $i$ is the time step number.

!---

## Theory: Element Size

There is no pre-defined measure of the element size.

Often alternatives for directly computing element information are used. For example, the square root
of the number of degrees-of-freedom can be used as a measure of element size for 2D regular grids.

!---

## Practice: Error

MOOSE includes a number of error calculations, for Lagrange shape functions the
$L_2$-norm computed at the nodes is adequate.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Postprocessors remove=Postprocessors/h

!---

## Practice: $\Delta t$

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_temporal.i link=False block=Postprocessors remove=Postprocessors/error

!---

## Practice: Element Size

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Postprocessors remove=Postprocessors/error


!---

## Practice: Spatial Convergence

!media mms_spatial.png style=width:70%;float:right;margin-left:2em;

Error decreases with decreasing element size, first-order elements converge with a slope of two and
second-order three, when plotted on a log-log scale.

!---

## Practice: Temporal Convergence

!media mms_spatial.png style=width:70%;float:right;margin-left:2em;

Error decreases with decreasing time step size, with the slope equal to the order of the method when
plotted on a log-log scale.
