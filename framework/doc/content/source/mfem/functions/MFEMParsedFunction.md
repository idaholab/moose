# MFEMParsedFunction

!if! function=hasCapability('mfem')

## Summary

!syntax description /Functions/MFEMParsedFunction

## Overview

`MFEMParsedFunction` is responsible for parsing expressions that, in addition
to being scalar functions of position and time, can also depend on scalar
coefficients, including any scalar variables, postprocessors, material
properties or functions specified by the user.

The input parameters are those of [ParsedFunction](/MooseParsedFunction.md)
and provide the same flexibility. Note that, in the context of MFEM problems,
a scalar variable is a real-valued scalar field, not (necessarily) a constant
over the entire domain as elsewhere in MOOSE.

## Example Input File Syntax

!listing test/tests/mfem/functions/parsed_function_source.i block=Functions

!syntax parameters /Functions/MFEMParsedFunction

!syntax inputs /Functions/MFEMParsedFunction

!syntax children /Functions/MFEMParsedFunction

!if-end!

!else
!include mfem/mfem_warning.md