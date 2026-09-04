# MFEMProblemComposer

!if! function=hasCapability('mfem')

## Overview

`MFEMProblemComposer` is the base class of those classes that build [ProblemOperator.md]
objects, through the method `createProblemOperator()`. The composer itself is created by
the [MFEMProblem.md].

These composer objects can be used to instantiate the provided equation system problem operators, or user defined 
custom ones. For example usage on custom operators refer to the walkthrough example in [this page](syntax/ProblemComposers/index.md).

!if-end!

!else
!include mfem/mfem_warning.md
