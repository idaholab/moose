# Application Development

These documentation pages are meant to be used by developers who are developing MOOSE-based applications.

## Documentation

[Modules](https://mooseframework.inl.gov/modules/index.html) - Physics and numerics modules to include in your application

[Syntax](syntax/index.md) - MOOSE syntax documentation

[Source Code](source/index.md exact=True) - MOOSE source documentation

[MOOSE Doxygen](https://mooseframework.org/docs/doxygen/moose/classes.html) - MOOSE Doxygen

[Modules Doxygen](https://mooseframework.inl.gov/docs/doxygen/modules/classes.html)

## Utilities

[Build System](/build_system.md) - How the hierarchical `make` system functions in MOOSE

[Test System](/test_system.md) - How to create/maintain tests for your application

[Performance Benchmarking](/performance_benchmarking.md) - How to perform benchmarking

[Profiling](/profiling.md) - How to profile your application in order to determine what functions are hogging compute time.

[Code Coverage](/coverage.md) - How to add automatic code coverage to your application, and use it in your development workflow

[Utils](utils/index.md) - General utilities used throughout the Framework and applications

[Python Tools](https://mooseframework.inl.gov/python/index.html) - Python tools to support development, verification, and others

## Development workflow

[Jacobian Definition](/jacobian_definition.md) - How to compute derivatives of your residual statements

[Development Tools](help/development/index.md) - Tools that can be helpful for development, like IDEs with MOOSE support

[Code Standards](sqa/framework_scs.md) - How we expect code to be formatted

[Debugging](application_development/debugging.md) - Tips on how to debug MOOSE-based applications

## MOOSE Systems

[Actions](source/actions/Action.md) - Objects used to execute various tasks

[Interfaces](framework_development/interfaces/index.md) - Base-classes that allow cross-cutting data retrieval

[RelationshipManagers](/relationship_managers.md) - Telling MOOSE about extra geometric or algebraic information needed in parallel

[Moose-Wrapped Apps](/moose_wrapped_apps.md) - Coupling external codes to MOOSE
