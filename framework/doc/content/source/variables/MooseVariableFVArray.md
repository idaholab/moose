# MooseVariableFVArray

!syntax description /Variables/MooseVariableFVArray

For an example of using FV array variables/kernels, see [syntax/FVKernels/index.md#FV Array Kernels]

An array variable is define as a set of standard field variables with the same
finite element family and order.  This support was originally developed for FE
variables, but has now been extended to finite volume (FV) variables.  The
same architecture used for FE array variables has been extended for FV.  For
details on how array variables are designed and work generally, refer to
[ArrayMooseVariable.md]. There are three primary differences with the FV array
variable implementation:

1. When using FV array variables, indexing into `_u`, `_grad_u`, etc. does
   not require a shape-function index `_i` - just directly start indexing with
   `_qp` just like with regular FV variables.


2. FV array variables operate with full automatic differentiation (AD)
   support/integration.  All material property and variable coupling should be
   via the corresponding AD-ified interfaces and types.  Users do not provide
   manual jacobian calculations.


3. FV array variables only work with global AD dof indexing (i.e. MOOSE must
   have been configured and compiled with the `--with-ad-indexing-type=global`
   flag.)

There may be some features that are not yet fully implemented.  Generally,
users should see explanatory error messages when they try to do something that
isn't yet fully implemented.

!syntax parameters  /Variables/MooseVariableFVArray

!syntax inputs /Variables/MooseVariableFVArray

!syntax children /Variables/MooseVariableFVArray
