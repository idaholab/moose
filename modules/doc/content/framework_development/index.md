# Framework Development

The articles attached to this page are useful if you intend on contributing to the MOOSE reposistory. Information about the repository
structure, code standards, testing, and software quality are all available here. If you are new to Git, we've created some information
about how to get up and running here as well.

For development of MOOSE-based applications see [Application Development](application_development/index.md).

## Overview on contributing

[Contributing](framework/contributing.md)

[Reviewing](framework/reviewing.md)

[How a patch becomes code](framework/patch_to_code.md)

[Code Standards](sqa/framework_scs.md) - How to format any code that goes into the framework

[Git](git.md) - The revision control system we use

[Development Tools](help/development/index.md) - Tools that can be helpful for development, like IDEs with MOOSE support

## Documentation

[MOOSE Doxygen](http://www.mooseframework.org/docs/doxygen/moose/classes.html)

[Modules Doxygen](https://mooseframework.inl.gov/docs/doxygen/modules/classes.html)

[libMesh Doxygen](https://mooseframework.org/docs/doxygen/libmesh/classes.html)

[TIMPI Doxygen](https://mooseframework.org/docs/doxygen/timpi/classes.html)

[Source Code Documentation](source/index.md exact=True)

[Syntax Documentation](syntax/index.md)

## Build Status and Automatic Metrics

[Build Status (external)](https://civet.inl.gov)

[Build Status (internal)](https://moosebuild.hpc.inl.gov)

[Code Coverage](http://mooseframework.org/docs/coverage/framework/)

[Test Timing](http://mooseframework.org/docs/timing/)

## Software Quality Assurance Documents

[sqa/index.md exact=True] - Landing page for the MOOSE (and MOOSE Modules) software quality assurance (SQA) pages

## Utilities

[/PerfGraph.md] - How to time sections of code in MOOSE

[MooseUtils Namespace](MooseUtils.md) - Basic utilities used throughout the framework

[Utils](utils/index.md) - Basic utilities used throughout the framework

[System Integrity Checking](sanity_checking.md) - Parsing and system integrity checks

## MOOSE Internal Systems

[Actions](source/actions/Action.md) - Objects used to execute various tasks

[Interfaces](framework_development/interfaces/index.md) - Base-classes that allow cross-cutting data retrieval

[MooseVariables](moose_variables.md) - The set of objects that compute and hold variable/field values

[Warehouses](/warehouses.md) - Objects that store all of the dynamically built MOOSE objects (`Kernels`, `BCs`, etc.)

[Tagging](tagging.md)

[RelationshipManagers](/relationship_managers.md) - Telling MOOSE about extra geometric or algebraic information needed in parallel

## Build System

[Working with MOOSE build system](build_system.md)

## Third Party Libraries

[Third Party Library List](sqa/library_requirements.md)
