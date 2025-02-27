# MFEMDataCollection

## Summary

Base class for MFEM DataCollection outputs.

## Overview

Virtual base class for classes that write out to `mfem::DataCollection` objects using MOOSE's
`Output` system. Output interval and triggers for writing to file can be controlled with the
`execute_on` and `interval` parameters, consistent with other MOOSE Outputs [as seen
here.](https://mooseframework.inl.gov/syntax/Outputs/index.html)

Child classes should override the `getDataCollection()` method to return a reference to the
`mfem::DataCollection` being written to at each output step.

## Example Input File Syntax

!listing test/tests/mfem/outputs/datacollections.i block=Outputs
