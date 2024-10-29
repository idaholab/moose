# MFEMCurlCurlKernel

## Summary

!syntax description /Kernels/MFEMCurlCurlKernel

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k\vec\nabla \times \vec u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$.

This term arises from the weak form of the curl curl operator

!equation
-k\vec\nabla \times \vec\nabla \times \vec u

## Example Input File Syntax

!listing curlcurl.i

!syntax parameters /Kernels/MFEMCurlCurlKernel

!syntax inputs /Kernels/MFEMCurlCurlKernel

!syntax children /Kernels/MFEMCurlCurlKernel
