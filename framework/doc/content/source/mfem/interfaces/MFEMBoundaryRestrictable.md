# MFEMBoundaryRestrictable

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM classes that are restricted to a subset of mesh boundaries.

## Overview

`MFEMBoundaryRestrictable` provides an interface for objects that can be restricted to one or more
boundary (sidesets) of the mesh, specified by the list of boundaries in the `boundary` input
parameter.

Boundaries may be specified either by their integer attribute ID or their name.

Child classes should access the array of boundary attributes and markers using the
`getBoundaryAttributes()` and `getBoundaryMarkers()` methods respectively.

!if-end!

!else
!include mfem/mfem_warning.md
