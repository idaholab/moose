# Abaqus UMAT Material

!syntax description /Materials/AbaqusUmatMaterial

## Description

`AbaqusUmatMaterial` provides an interface for using _Abaqus UMAT_
functions as constitutive models in MOOSE.

## UMAT User subroutine interface

UMAT functions are commonly coded in Fortran, are located in the
`moose/modules/tensor_mechanics/plugins`, and are automatically
compiled by the MOOSE build system.

A UMAT file `my_umat.f` can be loaded by the `AbaqusUmatMaterial` by providing the
the full path and filename without an extension via the [!param](/Materials/AbaqusUmatMaterial/plugin)
parameter.

A valid UMAT subroutine has the following call signature

```
SUBROUTINE UMAT(STRESS,STATEV,DDSDDE,SSE,SPD,SCD,
     1 RPL,DDSDDT,DRPLDE,DRPLDT,
     2 STRAN,DSTRAN,TIME,DTIME,TEMP,DTEMP,PREDEF,DPRED,CMNAME,
     3 NDI,NSHR,NTENS,NSTATV,PROPS,NPROPS,COORDS,DROT,PNEWDT,
     4 CELENT,DFGRD0,DFGRD1,NOEL,NPT,LAYER,KSPT,KSTEP,KINC)
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
can be found in the [Abaqus user manual](https://classes.engineering.wustl.edu/2009/spring/mase5513/abaqus/docs/v6.6/books/sub/default.htm?startat=ch01s01asb31.html)

!alert note
Temperature coupling and `PREDEF` support are not yet implemented.

## Example Input File

!listing modules/tensor_mechanics/test/tests/umat/linear_strain_hardening.i block=Materials/constant

!syntax parameters /Materials/AbaqusUmatMaterial

!syntax inputs /Materials/AbaqusUmatMaterial

!syntax children /Materials/AbaqusUmatMaterial

!bibtex bibliography
