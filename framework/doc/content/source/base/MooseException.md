# MooseException

MooseException is a normal C++ derived "exception" object. It's purpose is to give developers
an opportunity to terminate a solve, while informing MOOSE that it should perform the
necessary steps to clean up the current stack, notify other threads/processors of the error
and communicate with the solver that the current solve has failed. When applicable and possible,
the solver will allow MOOSE to cut the time step and make another attempt at a solve.

This exception should be used directly in user code when non-fatal situations are encountered
such as the inability to converge a local Newton solve in a material, or if a interpolated value
from a variable or lookup table ends up out-of-range.

MOOSE is setup to catch MooseExceptions inside of several user defined callbacks that occur during
the solve's computeResidual, and computeJacobian callbacks. Note that these callbacks also include
the evaluation of several other objects such as Materials, Constraints, and occasionally objects
such as Postprocessors (when requested). This makes it possible to try smaller solves from most
user-defined objects within MOOSE.

## How it works

MOOSE Wraps several threaded sections in PARALLEL_TRY/PARALLEL_CATCH macros (See SystemBase.h).
PARALLEL_TRY does nothing (but provide scope). PARALLEL_CATCH invokes the
FEProblem::checkExceptionAndStopSolve() (See FEProblemBase.h) routine after the threaded section
has terminated and all threads on the same processor agree on the error status.
checkExceptionAndStopSolve() then does parallel communication to ensure that all processors
are notified of a single failure and cutback together.
