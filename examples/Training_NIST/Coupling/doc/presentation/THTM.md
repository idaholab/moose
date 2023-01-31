# Coupling TH and TM

!---

## Step 1: Generating the mesh for the TH domain

!listing TH_TM_coupling/mesh_full.i

Run with:

```
blue_crab-opt -i mesh_full.i --mesh-only
```

This run produces the file ```mesh_full_in.e``` that we will read later in the application.

!---

## Step 2: Generating the mesh for the solid domain

!listing TH_TM_coupling/mesh_solid.i

Run with:

```
blue_crab-opt -i mesh_solid.i --mesh-only
```

This run produces the file ```mesh_solid_in.e``` that we will read later in the application.

!---

## Step 3: Thermal-hydraulics module

The thermal-hydraulics module is similar to the one that we have set up in the TH turbulent session.

!listing TH_TM_coupling/TH.i
         block=Mesh Modules Variables FVKernels AuxVariables AuxKernels FVInterfaceKernels FluidProperties Materials Executioner Debug Postprocessors Outputs

!---

## Step 4: Thermomechanics module

The thermomechanics module is similar to the one that we did in the TM session.

!listing TH_TM_coupling/TM.i

!---

## Step 5: Coupling

The coupling is performed via a MultiApp that sends the temperature and pressure fields into the mechanics calculations.

Note that sending back the displacements is deemed unnecessary since they will very mildly affect the flow field.

!listing TH_TM_coupling/TH.i
         block=MultiApps Transfers

!---

## Step 6: Running

To run the coupled system, we should run the main application as follows:

```
mpirun -np <N> blue_crab-opt -i TH.i
```

where ```<N>``` is the number of processors.

The sub-application ```TM.i``` is detected automatically from the ```MultiApp```.
