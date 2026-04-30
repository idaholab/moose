# GravityVectorInterface

This interface provides input parameters to specify the gravity vector and two methods:

- `gravityVector()` returns the gravity vector.
- `gravityMagnitude()` returns the magnitude of the gravity vector.
- `gravityDirection()` returns the unit direction vector of the gravity vector. If
  the magnitude of gravity is zero, this will return the zero vector.
