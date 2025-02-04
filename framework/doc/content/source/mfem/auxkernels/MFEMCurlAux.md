# MFEMCurlAux

## Summary

!syntax description /AuxKernels/MFEMCurlAux

## Overview

AuxKernel for calculating the curl of an $H(\mathrm{curl})$ conforming source variable defined on a 3D ND FE
space and storing it in a $H(\mathrm{div})$ conforming result variable defined on a RT FE space.

The result may be scaled by an optional (global) scalar factor.

!equation
\vec v =  \lambda \vec\nabla \times \vec u

where $\vec u \in H(\mathrm{curl})$, $\vec v \in H(\mathrm{div})$ and $\lambda$ is a scalar constant.

## Example Input File Syntax

!listing curlcurl.i

!syntax parameters /AuxKernels/MFEMCurlAux

!syntax inputs /AuxKernels/MFEMCurlAux

!syntax children /AuxKernels/MFEMCurlAux
