# [Reporter System](syntax/Reporters/index.md)

An advanced system for postprocessing. It is based on a producer/consumer paradigm. Objects
may produce reporters, which they declare to the Problem, and other objects may retrieve them from the Problem.

The reporter data can be of very many different types, from a single scalar to maps, vectors and points.

Vectorpostprocessors are an example of reporters, using `std::vector<Real>` for the reporter data.
Reporters include automatic json output.

!---
