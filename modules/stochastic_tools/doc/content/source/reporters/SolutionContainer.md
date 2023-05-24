# SolutionContainer

!syntax description /Reporters/SolutionContainer

## Overview

This object is responsible for accumulating solution fields over the course of a simulation.
The solution fields are kept distributed using the communicator of the application.

## Example Input File Syntax

!listing test/tests/reporters/parallel_storage/sub.i block=Reporters

## Syntax

!syntax parameters /Reporters/SolutionContainer

!syntax inputs /Reporters/SolutionContainer

!syntax children /Reporters/SolutionContainer
