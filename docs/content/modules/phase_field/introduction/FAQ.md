# Frequently Asked Questions

## What is the relationship between interface width and mesh/grid spacing in phase-field models?

It is important to understand the difference between interface width and grid
spacing in phase-field models. The interface width is a function of model parameters
that enter the governing equations, and is not dependent on the grid spacing or
other details of the discretization. For example, in the most basic Cahn-Hilliard
model, the interface width is a function of the gradient energy coefficient $\kappa$
and the height of the free energy barrier between equilibrium phases. When you change
the grid spacing and keep the system dimensions the same, you change the number of
elements in the interfacial region, but not the width of the interface in the
coordinate system you have chosen. So, if you were to keep the system dimensions
the same, simulate an interface between two phases with increasingly finer resolution,
and plot the results on top of one another, you would see the same interface shape
(width) represented with an increasing number of data points in the interfacial
region, but the interface width in your coordinate system would not change.

## How do I know if the mesh spacing I am using in my phase-field simulation is fine enough?

If you keep the governing equations of the model the same, as you make the mesh finer,
as long as the number of elements in the interface is high enough, you should get the
same physical results. For the basic Cahn-Hilliard model I referred to earlier, a good
rule of thumb would be that you would want at least 4-5 elements in the interface
(defined as $0.1 < c < 0.9$ if the equilibrium values of $c$ are 0 and 1) if you are
using linear Lagrange elements. You may be able to get away with fewer elements than
that, and if you are looking only at qualitative differences after a few time steps,
you may not notice any changes. But if you want to lower resolution below the 4-5
elements through the interface that I mentioned you probably should plot the system
energy as a function of time to verify decreasing resolution is not affecting the
system's evolution.
