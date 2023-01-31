# Physics Coupling in MOOSE

!---

## Why Multiphysics Coupling?

As we have seen, the ```MultiApp``` systems allows us to couple phsyics appications within MOOSE. The coupling can be also extended to external solvers as needed.

Coupling analyses are needed when:

- The behavior of the system cannot be characterized by *freezing* some of the physics, e.g., there is a considerable power shift with temperature, which, in turn, affects he thermal fields.

- It is unclear what are the conservative conditions during a transient, e.g., in a loss of flow transient it may be not clear at what time in the transient the maximum temperature in the fuel is reached.

- The analyst seeks to resude uncertainties in the analysis coming from the multiphysics coupling of the fields.

!---

## MOOSE approach to Multiphysics Coupling

- In general, multiphysics coupling is desirable as less approximations must be made by the analyst.

- Other available commercial code couplings, e.g., Star-CCM+-Abaqus, ANSYS-modules, etc., rely on external file coupling or memory sockets implemented in the code.

- This coupling is frequently enough when coupling simple scalar quantities, e.g., the average pressure between a fluid and a solid domain.

- However, these couplings are very memory intensive and "dangerous" when coupling fields since it assumes akin memory structures and writting schedulers on the coupled codes.

- MOOSE objective is to make coupling seamless, robust, and fast across physics and scales.

!---

## MOOSE approach to Multiphysics Coupling (Cntd.)

- For this purpose:

  - All physics in MOOSE are implemented across the same memory structures
  - The coupling process can be tailored and controlled via the native ```MultoApp``` system
  - MOOSE supports distributed mesh parallelism when coupling physics

!---

## Coupling physics for the flow between plates example

- The first part of the hands-on session will deal with coupling the TH to the TM model

- Then, in the second part of the hands-on session, we will couple neutronics to the TH + TM model
