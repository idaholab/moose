# ParsedVectors

!syntax description /Reporters/ParsedVectors

## Overview

`ParsedVectors` operates on the elements contained in series of vector reporters using a [parsed function expression](MooseParsedFunction.md) as shown in [vectorOperation].  This reporter allows several vector reporters to be combined into single vector.  This reporter operates on multiple vectors of the same size and returns a vector of that size.  There must be one [!param](/Reporters/ParsedVectors/reporter_names) for each [!param](/Reporters/ParsedVectors/reporter_symbols).
This is a vector version of the [ParsedScalars.md] reporter.

!listing modules/optimization/test/tests/reporters/vector_math/vectorMath.i id=vectorMath
block=Reporters/vecs Reporters/vectorOperation

!syntax parameters /Reporters/ParsedVectors

!syntax inputs /Reporters/ParsedVectors

!syntax children /Reporters/ParsedVectors
