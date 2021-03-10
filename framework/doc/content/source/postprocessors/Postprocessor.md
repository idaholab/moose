# Postprocessors

The Postprocessor object is the pure-virtual base class for defining all Postprocessors in MOOSE. It defines a single
method that all Postprocessors must override to produce a global scalar value. This method is generally called and stored
in multiple [Output](/Output.md) formats once per time step (when applicable).

!listing framework/include/postprocessors/Postprocessor.h
  re=([^\n]+\n)*[^\n]+getValue[^\n]*;
