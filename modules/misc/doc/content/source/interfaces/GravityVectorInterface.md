# GravityVectorInterface

This interface is used to specify the gravity vector. There are two options:

- Provide the direction via `gravity_direction`. The magnitude may optionally be specified using `gravity_magnitude`; else it defaults to the exact NIST constant for the acceleration due to gravity, 9.80665 m/s$^2$.
- Provide the gravity vector via `gravity_vector`.

An error will result for any of the following cases:

- Both `gravity_vector` and `gravity_direction` are specified.
- Both `gravity_vector` and `gravity_magnitude` are specified.
- Neither `gravity_direction` nor `gravity_vector` are specified.
