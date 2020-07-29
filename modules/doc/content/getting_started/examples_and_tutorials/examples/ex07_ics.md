# Example 07 : Custom Initial Conditions

MOOSE provides several ways to specify initial conditions (ICs) for transient problems.  The same
functionality can be used to specify initial guesses for steady-state problems.  Custom methods
can be created in addition to using any of many built-in methods for specifying initial
conditions.  Detailed information for all the built-in initial conditions available can be found
on the [Initial Condition System Documentation](syntax/ICs/index.md)

This example demonstrates creating and using a custom class+object for setting the following
initial condition:

\begin{equation}
\begin{aligned}
    u(t=t_0, x, y, z) = 2 C x
\end{aligned}
\end{equation}

where $C$ is a user-chosen coefficient. To accomplish this, we will need to create our own
subclass of MOOSE's `InitialCondition` class and then specify the IC in our input file.

## Creating a Custom IC

We create a header file and subclass the `InitialCondition` class and add a `Real` (i.e. floating
point number) member variable to hold the user-specified coefficient - see
[examples/ex07_ics/include/ics/ExampleIC.h] for details. The `.C` file defines an input parameter
named `coefficient`, stores its value in a member variable in the constructor, and uses that value
to compute the IC.

!listing examples/ex07_ics/src/ics/ExampleIC.C max-height=10000

## Using ICs in an Input File

The input file [examples/ex07_ics/transient.i] sets up a simple transient diffusion problem with
initial conditions specified in the `Variables` block:

!listing examples/ex07_ics/transient.i block=Variables

We can also use the initial condition when running steady-state problems (i.e. with the `Steady`
Executioner); this effectively functions as an initial guess for the solver - usually not
necesary, but occasionally useful.  For steady cases, the IC is specified in exactly the same way
- see e.g. [examples/ex07_ics/steady.i].

# Results

These results are from running the `transient.i` file. At $t=0$, we can see that the initial
condition defines the solution to be zero at $x=0$ (along the rear surface of the half-cylinder)
and increasing linearly toward us (out of the page).  At the final time, we see that diffusion has
overcome our IC and our Dirichlet BCs have dictated the solution along the top and bottom
surfaces.

!media large_media/examples/ex07/transient_t0.png
       style=width:33%;display:inline-flex;
       caption=Initial state (t = 0)

!media large_media/examples/ex07/transient_tmid.png
       style=width:33%;display:inline-flex;
       caption=Intermediate diffused state (t = .1)

!media large_media/examples/ex07/transient_tend.png
       style=width:33%;display:inline-flex;
       caption=Final equilibrium state (t = 1 )

## Complete Source Files

- [examples/ex07_ics/include/ics/ExampleIC.h]
- [examples/ex07_ics/src/ics/ExampleIC.C]
- [examples/ex07_ics/transient.i]
- [examples/ex07_ics/steady.i]

!content pagination use_title=True
                    previous=examples/ex06_transient.md
                    next=examples/ex08_materials.md
