# AuxiliarySystem

The AuxiliarySystem object is used by MOOSE to hold field variables that are not part of the
PDE system that you are solving for. These are explicitly "calculated" quantities that have
full shape function support, access to previous values (old and older).