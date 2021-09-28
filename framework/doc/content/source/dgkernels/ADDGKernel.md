# ADDGKernel

Base class for all DG kernels making use of automatic differentiation.

This class takes care of accumulating the residual and Jacobian contributions, including
the neighbor and off-diagonal contributions.
