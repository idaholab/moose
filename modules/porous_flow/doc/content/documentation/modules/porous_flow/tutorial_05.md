[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_04.md) |
[Next](porous_flow/tutorial_06.md)

# Porous Flow Tutorial Page 05.  Using a realistic equation of state for the fluid

It is trivial to add a realistic equation of state to any PorousFlow simulation.  For instance, the high-precision [`Water97`](Water97FluidProperties.md) equation of state may be used:

!listing modules/porous_flow/examples/tutorial/05.i start=[Modules] end=[Materials]

(The name "the_simple_fluid" could also be changed to something more appropriate.)

In this case, the initial and boundary conditions must also be changed because the Water97 equation of state is not defined for $P=0$ (an absolute vacuum).  For example:

!listing modules/porous_flow/examples/tutorial/05.i start=[Variables] end=[PorousFlowBasicTHM]

and

!listing modules/porous_flow/examples/tutorial/05.i start=[BCs] end=[Modules]

Using realistic high-precision equations of state can cause PorousFlow to run quite slowly, because the equations of state are so complicated to evaluate.  It is always recommended to use [`TabulatedFluidProperties`](TabulatedFluidProperties.md) in the following way:

!listing modules/porous_flow/examples/tutorial/05_tabulated.i start=[Modules] end=[Materials]

and

!listing modules/porous_flow/examples/tutorial/05_tabulated.i start=[PorousFlowBasicTHM] end=[BCs]

Another advantage of using [`TabulatedfluidProperties`](TabulatedFluidProperties.md) is that the bounds on pressure and temperature imposed by the original "true" equation of state can be extrapolated past, which can help to remove problems of MOOSE trying to sample outside the original's region of validity (and thus causing the timestep to be cut).

Finally, in the case at hand, it is worth reminding the reader that `PorousFlowBasicTHM` is based on the assumptions of a constant, large fluid bulk modulus, and a constant fluid thermal expansion coefficient, which are incorrect for some fluids.  User beware!

[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_04.md) |
[Next](porous_flow/tutorial_06.md)
