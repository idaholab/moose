# ProblemComposerBase

!if! function=hasCapability('mfem')

## Summary

`ProblemComposerBase` is the base class of those classes that build [ProblemOperator.md]
objects, e.g. when called by an [MFEMSteady.md] or [MFEMTransient.md] executioner object, through the method `createProblemOperator()`.
The composer itself is created by and stored in the [MFEMProblem.md].

These composer objects can be used to instantiate the provided equation system problem operators, or user defined 
custom ones. For example usage on custom operators refer to the walkthrough example in [this page](syntax/ProblemComposer/index.md).

!if-end!

!else
!include mfem/mfem_warning.md
