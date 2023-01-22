# Hands-on session

### Thermomechanical analysis the flow between parallel plates

!---

## Building up the model step-by-step

!row!
!col! width=50%

### Steps:

- *Step 1:* Heat conduction in the plates

- *Step 2:* Step 1 + coupling with elastic mechanics solve

- *Step 3:* Step 2 + adding parsed material properties

- *Step 4:* Step 3 + adding realistic heat source in the fuel

- *Step 5:* Step 4 + adding plasticity

- *Step 6:* Step 4 + using coded BISON materials

!col-end!

!col! width=40%

!media plates_hands_on.png style=width:80%;

!col-end!

!row-end!

!---

## *Step 1:* Heat conduction in the plates

!listing heat_conduction_mechanics_mats_source.i

!listing bending_3pt_3d.i


!---

