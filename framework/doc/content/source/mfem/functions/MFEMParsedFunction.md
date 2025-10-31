# MFEMParsedFunction

!if! function=hasCapability('mfem')

## Summary

!syntax description /Functions/MFEMParsedFunction

## Overview

`MFEMParsedFunction` is responsible for parsing expressions that, in addition to being functions of position and time, can also depend on problem variables prescribed by input parameters.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion_parsed_source.i block=Functions AuxVariables

!syntax parameters /Functions/MFEMParsedFunction

!syntax inputs /Functions/MFEMParsedFunction

!syntax children /Functions/MFEMParsedFunction

!if-end!

!else
!include mfem/mfem_warning.md