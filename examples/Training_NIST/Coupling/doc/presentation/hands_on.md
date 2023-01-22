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

To generate the mesh run:

```
<exec>-opt -i mesh.i --mesh-only
```

where you replace ```<exec>``` by the name of your executable file, e.g., ```bison```, ```moose```, ```blue_crab```, etc.

The heat conduction case in the plates is similar to the one we have done in the previous tutorials:

!listing heat_conduction_plates.i

!---

## *Step 2:* Step 1 + coupling with elastic mechanics solve

Now we add elastic thermomechanics. Key new additions to notice:

- Restarting temperature field from the previous heat conduction calculation
- Adding ```TensorMechanics``` action + mechanics materials
- Adding post-processors to evaluate 3D plate displacement

!listing heat_conduction_mechanics.i

!---

## *Step 3:* Step 2 + adding parsed material properties

Next, we add fitted temperature-dependent material properties:

!listing heat_conduction_mechanics_mats.i
         block=Functions SolidProperties Materials

!---

## *Step 4:* Step 3 + adding realistic heat source in the fuel

Following, we add a custom fitted function for the power density:

!media power_density_history.png style=width:80%

!---

## *Step 4:* Step 3 + adding realistic heat source in the fuel

The code:

!listing heat_conduction_mechanics_mats_source.i
         block=Functions Kernels

!---

## *Step 5:* Step 4 + adding plasticity

Next, we add a pressure difference between the faces and account for plastic deformation of the clad:

!listing heat_conduction_plastic_mats_source.i
         block=Materials BCs

!---

## *Step 6:* Step 4 + using coded BISON materials

Finally, let's analyze a complete example using BISON materials:

!listing bison_mechanics.i

!---

