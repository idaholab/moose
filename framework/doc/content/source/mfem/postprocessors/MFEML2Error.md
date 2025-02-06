# MFEML2Error

## Summary

!syntax description /Postrocessors/MFEML2Error

## Overview

Postprocessor for calculating the L2 error of a scalar $H^1$ or $L^2$
conforming source variable compared to a scalar function.

!equation
\left\Vert u_{ex} - u_{h}\right\Vert_{\mathrm{L2}}

where $u_{h} \in H^1 \lor L^2$ and $u_{ex}$ is a scalar function.

## Example Input File Syntax

!listing kernels/irrotational.i

!syntax parameters /Postrocessors/MFEML2Error

!syntax inputs /Postrocessors/MFEML2Error

!syntax children /Postrocessors/MFEML2Error
