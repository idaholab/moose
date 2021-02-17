# GeometricCutSubdomainIDAux

!syntax description /AuxKernels/GeometricCutSubdomainIDAux

## Overview

To allow using XFEM to model interfaces between materials, the XFEM system uses geometric cut subdomain IDs to denote the subset of a standard MOOSE subdomain (element block) that a material point belongs to. `GeometricCutSubdomainIDAux` outputs the value of the geometric cut subdomain ID for each element.

## Example Input File Syntax

!syntax parameters /AuxKernels/GeometricCutSubdomainIDAux

!syntax inputs /AuxKernels/GeometricCutSubdomainIDAux

!syntax children /AuxKernels/GeometricCutSubdomainIDAux
