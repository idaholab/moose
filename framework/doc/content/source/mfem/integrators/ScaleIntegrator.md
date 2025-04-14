# ScaleIntegrator

!if! function=hasCapability('mfem')

## Summary

Forms an [`mfem::BilinearFormIntegrator`](https://mfem.org/bilininteg/) equal to a given
[`mfem::BilinearFormIntegrator`](https://mfem.org/bilininteg/) multiplied by a real constant.

## Overview

Defines an [`mfem::BilinearFormIntegrator`](https://mfem.org/bilininteg/) defined from another
[`mfem::BilinearFormIntegrator`](https://mfem.org/bilininteg/) multiplied by a real constant. Used
for scaling existing integrators by a timestep in transient solves.

!else
!include mfem/mfem_warning.md
