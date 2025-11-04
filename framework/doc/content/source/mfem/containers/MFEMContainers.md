# NamedFieldsMap

!if! function=hasCapability('mfem')

## Summary

Adapted map for returning shared pointers to objects from string names.

## Overview

`NamedFieldsMap` is an adapted `std::map` used to register and retrieve shared pointers of MFEM
objects using field names (strings) as keys.

!if-end!

!else
!include mfem/mfem_warning.md
