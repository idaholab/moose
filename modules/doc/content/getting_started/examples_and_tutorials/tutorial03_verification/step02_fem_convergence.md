# FEM Convergence

## Introduction

The temporal numerical integration methods as well as the [!ac](FEM) have known rates of
convergence to the exact solution for changes in the time step and element size.
The rates are exploited to verify that a simulation is performing as expected. The known
convergence rates and the associated theory are well documented---e.g., [!cite](fish2007first)---the
reader is encouraged to research the theory behind these rates. The following sections briefly
discuss the known convergence rates.

## Error Source

For a typical [!ac](FEM) simulation there are two primary sources of error: (1) error from the [!ac](FEM)
approximation and (2) error from the time integration. Of course, there are other source of
error such as floating point arithmetic and model simplifications. In the first case, the error
associated with the [!ac](FEM) approximation goes to zero as the element size goes to zero. Similarly,
as the time step in the numerical integration goes to zero so does the associated error.

## Spatial Convergence

Spatial convergence for the [!ac](FEM) is defined here as the rate of error reduction with decreasing
finite element size. The rate is dependent on the order of the shape functions and is typically
reported as the slope of a line that compares error (y-axis) and element size (x-axis) on a
log-log plot. First-order shape functions should have a slope of two and second-order a slope of
three. For example, [tutorial03-spatial-example], is an example convergence plot for a
simulation with first-order and second-order shape functions, where the computed error is the
$L_2$ norm.

!media mms_spatial.png id=tutorial03-spatial-example caption=Example results for spatial convergence study.

!alert tip title=Eliminate temporal error if possible
When performing a spatial convergence study of a transient simulation, it is best to design the simulation such
that error associated with numerical integration is eliminated or minimal.

## Temporal Convergence

Temporal convergence is defined here as the rate of error reduction with decreasing the time step
size. The rate is proportional to the order of the numerical integration scheme selected and
typically reported as the slope of a line that compares the error (y-axis) and time step size
(x-axis) on a log-log plot. This slope is expected to be one for first-order schemes, two for
second-order schemes, etc. For example, [tutorial03-temporal-example], is an example convergence plot
for a simulation with first-order and second-order time integration schemes, where the
computed error is the $L_2$ norm.

!media mms_temporal.png id=tutorial03-temporal-example caption=Example results for temporal convergence study.

!alert tip title=Eliminate spatial error if possible
When performing a temporal convergence study, it is best to design the simulation such
that error associated with the [!ac](FEM) approximation is eliminated or minimal.


!bibtex bibliography !!include to make sure next/previous are last on page

!content pagination previous=tutorial03_verification/step01_heat_conduction.md
                    next=tutorial03_verification/step03_analytical_solution.md
