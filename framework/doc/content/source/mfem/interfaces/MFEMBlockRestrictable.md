# MFEMBlockRestrictable

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM classes that are restricted to a subset of mesh subdomains.

## Overview

`MFEMBlockRestrictable` provides an interface for objects that can be restricted to one or more
subdomains of the mesh, specified by the list of subdomains in the `block` input parameter.

Subdomains may be specified either by their integer attribute ID or their name.

Child classes should access the array of subdomain attributes and markers using the
`getSubdomainAttributes()` and `getSubdomainMarkers()` methods respectively.

!if-end!

!else
!include mfem/mfem_warning.md
