# MFEMParsedFunction

!if! function=hasCapability('mfem')

## Summary

!syntax description /Functions/MFEMParsedFunction

## Overview

`MFEMParsedFunction` is responsible for parsing expressions based on names and values of variables prescribed by input parameters.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion_parsed_source.i block=Functions AuxVariables

!syntax parameters /Functions/MFEMParsedFunction

!syntax inputs /Functions/MFEMParsedFunction

!syntax children /Functions/MFEMParsedFunction

!if-end!

!else
!include mfem/mfem_warning.md