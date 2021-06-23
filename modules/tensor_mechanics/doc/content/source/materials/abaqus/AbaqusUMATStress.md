# Abaqus UMAT Stress

!syntax description /Materials/AbaqusUMATStress

## Description

`AbaqusUMATStress` provides an interface for using _Abaqus UMAT_
functions as constitutive models in MOOSE.

## UMAT User subroutine interface

UMAT functions are commonly coded in Fortran, are located in the
`$(APPLICATION_DIR)/plugins` and `$(APPLICATION_DIR)/test/plugins` directories,
and are automatically compiled by the MOOSE build system.

A UMAT file `my_umat.f` can be loaded by the `AbaqusUMATStress` by providing the
the full path and filename without an extension via the
[!param](/Materials/AbaqusUMATStress/plugin) parameter in the MOOSE input file.

A valid UMAT subroutine has the following call signature

```
SUBROUTINE UMAT(STRESS,STATEV,DDSDDE,SSE,SPD,SCD,
     1 RPL,DDSDDT,DRPLDE,DRPLDT,
     2 STRAN,DSTRAN,TIME,DTIME,TEMP,DTEMP,PREDEF,DPRED,CMNAME,
     3 NDI,NSHR,NTENS,NSTATV,PROPS,NPROPS,COORDS,DROT,PNEWDT,
     4 CELENTCELENT,DFGRD0,DFGRD1,NOEL,NPT,LAYER,KSPT,KSTEP,KINC)
C
      CHARACTER*80 CMNAME
      DIMENSION STRESS(NTENS),STATEV(NSTATV),
     1 DDSDDE(NTENS,NTENS),DDSDDT(NTENS),DRPLDE(NTENS),
     2 STRAN(NTENS),DSTRAN(NTENS),TIME(2),PREDEF(1),DPRED(1),
     3 PROPS(NPROPS),COORDS(3),DROT(3,3),DFGRD0(3,3),DFGRD1(3,3)


      user coding to define DDSDDE, STRESS, STATEV, SSE, SPD, SCD
      and, if necessary, RPL, DDSDDT, DRPLDE, DRPLDT, PNEWDT


      RETURN
      END
```

A description of the input and output parameters of the UMAT user subroutines
can be found in the Abaqus user manual.

## UMAT Time step control

Time step control within the UMAT subroutine is enriched with MOOSE's general capabilities. `PNEWDT` denotes a factor that can be modified in the UMAT routine to be less than one, to signal a cut in the time step, or more than one, to signal a local increment in the time step. Since these time step controls are local to the quadrature point, MOOSE provides a layer of objects to handle the information provided by UMAT's `PNEWDT`. First, we select a soft `Terminator` which will invalidate a time step if the time step increment used turns out to be larger than a computed maximum time step increment anywhere in the system:

!listing modules/tensor_mechanics/test/tests/umat/time_step/elastic_timestep.i block=UserObjects

As a second step, we select the time stepper controls for the entire finite element system. These controls will apply to UMAT stepping as well as any other kernel or object in the system:

!listing modules/tensor_mechanics/test/tests/umat/time_step/elastic_timestep.i block=Executioner/TimeStepper

The time step increment will be reduced as prescribed in the UMAT routine. However, the maximum increment will be limited by the selection of `growth_factor`. This avoids increasing the time step too much from one time step to the next. To further rely on UMAT's `PNEWDT`, one can choose to select a larger `growth_factor`.

A subroutine that acts on `PNEWDT` is chosen in the regression test:

!listing modules/tensor_mechanics/test/tests/umat/time_step/elastic_timestep.i block=Materials/umat


## Example input file

!listing modules/tensor_mechanics/test/tests/umat/elastic_hardening/linear_strain_hardening.i block=Materials/constant

!syntax parameters /Materials/AbaqusUMATStress

!syntax inputs /Materials/AbaqusUMATStress

!syntax children /Materials/AbaqusUMATStress

!bibtex bibliography
