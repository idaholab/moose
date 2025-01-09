# PlatypusApp

## Summary

The `PlatypusApp` object is the base class for Platypus and serves as the entry point for running an MFEM problem in MOOSE.

## Overview

`PlatypusApp` extends the functionality of `MooseApp` by declaring task dependencies between
MFEM-specific steps of the MFEM problem assembly. Dependencies are respected when parsing the user's
input file for Platypus, so objects will be constructed in the correct order. For example, this is where
it is enforced that the build of `MFEMFESpace` objects occurs before the build of `MFEMVariable`
objects that may depend on them.

!listing src/base/PlatypusApp.C
