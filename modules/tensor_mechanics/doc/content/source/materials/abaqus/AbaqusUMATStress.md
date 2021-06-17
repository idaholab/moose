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

!alert note
Temperature coupling and `PREDEF` support are not yet implemented.

## Example Input File

!listing modules/tensor_mechanics/test/tests/umat/linear_strain_hardening.i block=Materials/constant

!syntax parameters /Materials/AbaqusUMATStress

!syntax inputs /Materials/AbaqusUMATStress

!syntax children /Materials/AbaqusUMATStress

!bibtex bibliography
