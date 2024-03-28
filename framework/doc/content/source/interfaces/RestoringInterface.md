# RestoringInterface

The RestoringInterface is used to define the object's behavior when the problem is restoring to its previous state in time.
The problem may be restoring when a solve fails.

Ideally, if an object is able to modify the "state" of the problem in a transient simulation, the object is then also
responsible for defining the correct behavior when the problem is restoring. The state of the problem could include any of the following

- Adaptivity
- Stateful material properties
- Mesh changes

The framework handles adaptivity and stateful property restoration properly, and so the object does not need to account
for them in the `restoringProblem` API (explained below).

## End Use API

An example usage of the interface can be found in

!listing AddOnePostprocessor.h

!listing AddOnePostprocessor.C
