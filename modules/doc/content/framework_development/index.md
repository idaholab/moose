# Framework Development

The articles attached to this page are useful if you intend on contributing to the MOOSE reposistory. Information about the repository
structure, code standards, testing, and software quality are all available here. If you are new to Git, we've created some information
about how to get up and running here as well.

For development of MOOSE-based applications see [Application Development](application_development/index.md).

## Overview on contributing

[Contributing](framework/contributing.md)

[How a patch becomes code](framework/patch_to_code.md)

[Code Standards](sqa/framework_scs.md) - How to format any code that goes into the framework

## Git Tips

[Git](git.md) - The revision control system we use

## Documentation

[Doxygen](http://www.mooseframework.com/docs/doxygen/moose/classes.html)

[Source Code Documentation](source/index.md exact=True)

[Syntax Documentation](syntax/index.md)

## Build Status and Automatic Metrics

[Build Status (external)](https://civet.inl.gov)

[Build Status (internal)](https://moosebuild.hpc.inl.gov)

[Code Coverage](http://mooseframework.com/docs/coverage/moose/)

[Test Timing](http://mooseframework.org/docs/timing/)

## Moose Interfaces

[Interfaces](framework_development/interfaces/index.md) - Base-classes that allow cross-cutting data retrieval

## Utilties

[/PerfGraph.md] - How to time sections of code in MOOSE

[MooseUtils Namespace](MooseUtils.md) - Basic utilities used throughout the framework

[Utils](utils/index.md) - Basic utilities used throughout the framework

[System Integrity Checking](sanity_checking.md) - Parsing and system integrity checks

## Internal Systems

[MooseVariables](moose_variables.md) - The set of objects that compute and hold variable/field values

[Warehouses](/warehouses.md) - Objects that store all of the dynamically built MOOSE objects (`Kernels`, `BCs`, etc.)

[Code Standards](sqa/framework_scs.md)

[Tagging](tagging.md)

## Build System

[Working with MOOSE build system](build_system.md)

## Third Party Libraries

[Third Party Library List](sqa/library_requirements.md)
