# Non-overlapping coupling

Non-overlapping coupling refers to coupling two flow simulations at their boundaries.
Sufficient information is transferred to characterize the mass, momentum and energy flux at
the inlet of one simulation and the outlet of the other. 

This usually means transferring the inlet pressure of each simulation and impose at the outlet of the other.
Similarly, the outlet velocity and temperature, or mass flow rate and temperature, is transferred from the outlet of
each simulation to the inlet of the other simulation. This is done by leveraging the [Transfers system](syntax/Transfers/index.md).