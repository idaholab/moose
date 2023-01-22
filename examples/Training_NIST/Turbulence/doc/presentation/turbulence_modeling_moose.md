# Coupling TH and TM

### Part 1: Steady-State Coupling

!---

# Coupling scheme

!media all_coupling.png

!---

## Part 1 - Step 1: Generating the mesh for the TH + neutronics domains

!listing coupled_all_steady/mesh_full.i

Run with:

```
blue_crab-opt -i mesh_full.i --mesh-only
```

This run produces the file ```mesh_full_in.e``` that we will read later in the application.

!---

## Part 1 - Step 2: Generating the mesh for the solid domain

!listing TH_TM_coupling/mesh_solid.i

Run with:

```
blue_crab-opt -i mesh_solid.i --mesh-only
```

This run produces the file ```mesh_solid_in.e``` that we will read later in the application. Note: the mesh generation app run fast on these examples. If more complicated meshes are needed, one can directly reference the mesh file without the need of re-generating the mesh.

!---

## Part 1 - Step 3: Neutronics model

The neutronics model is the 2-group diffusion model that we have previously developed.

!listing coupled_all_steady/neutronics.i
         block=Mesh TransportSystems AuxVariables GlobalParams Materials PowerDensity Executioner Debug Outputs

!---

## Part 1 - Step 4: TH model

The TH model is equivalent to the one that we used in the TH-TM coupling.

!listing coupled_all_steady/TH.i
         block=Mesh Modules Variables FVKernels AuxVariables AuxKernels FVInterfaceKernels FluidProperties Materials Executioner Debug Postprocessors Outputs

!---

## Part 1 - Step 5: TM model

The TM model is equivalent to the one that we used in the TH-TM coupling.

!listing coupled_all_steady/TM.i

!---

## Part 1 - Step 6: Coupling neutronics to TH

The coupling from neutronics to TH is performed via a ```FullSolveMultiApp```

!listing coupled_all_steady/neutronics.i
         block=MultiApps Transfers

!---

## Part 1 - Step 7: Coupling TH to TM

The coupling from TH to TM is also performed via a ```FullSolveMultiApp```

!listing coupled_all_steady/TH.i
         block=MultiApps Transfers

!---

### Part 1 - Step 8: Running the model

To run the coupled system, we should run the main application as follows:

```
mpirun -np <N> blue_crab-opt -i neutronics.i
```

where ```<N>``` is the number of processors.

The sub-applications ```TH.i``` and ```TM.i``` are detected automatically from the ```MultiApps``` system.

!---

# Coupling TH and TM

### Part 2: Transient Coupling

!---

## Part 2 - Step 1: Transient neutronics model

A transient neutronics model is initialized from the steady-state:

!listing coupled_all_transient/neutronics.i

!---

## Part 2 - Step 2: Transient TH model

A matching TH model is initialized from the steady-state:

!listing coupled_all_transient/TH.i

!---

## Part 2 - Step 3: TM model

The TM model is also initialized from the steady-state result:

!listing coupled_all_transient/TM.i

Note: we are neglecting the acoustic response of the fuel.

!---

### Part 2 - Step 4: Running the model

To run the coupled system, we should run the main application as follows:

```
mpirun -np <N> blue_crab-opt -i neutronics.i
```

where ```<N>``` is the number of processors.

The sub-applications ```TH.i``` and ```TM.i``` are detected automatically from the ```MultiApps``` system.
