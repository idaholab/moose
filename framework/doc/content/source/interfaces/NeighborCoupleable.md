# NeighborCoupleable

This class provides API for coupling different kinds of variables values into MOOSE systems which
are performing computations on element internal sides.
This class derives from [Coupleable.md] so it includes all the APIs for coupling in
variable values in the local element. It add the following APIs for the variable values in the
neighbor.

| Method | Description |
| - | - |
coupledNeighborValue | Values of a coupled variable at neighbor q-points
coupledNeighborGradient | Gradients of a coupled variable at neighbor q-points
coupledNeighborSecond | Second derivatives of a coupled variable at neighbor q-points
adcoupledNeighborValue | Values of a coupled variable at neighbor q-points with automatic differentiation info
adcoupledNeighborGradient | Gradients of a coupled variable at neighbor q-points with automatic differentiation info
adcoupledNeighborSecond | Second derivatives of a coupled variable at neighbor q-points with automatic differentiation info


For values, gradients and second derivatives, users can request old and older values in case they are running a transient simulation.
In case of old and older values, the methods are called `coupledNeighborValueOld` and `coupledNeighborValueOlder`, respectively.

# Other APIs

See the [Coupleable.md] documentation for:

- checking whether a variable is coupled in
- coupling in vector variables
- accessing degrees-of-freedom values directly
- writing to coupled variables
