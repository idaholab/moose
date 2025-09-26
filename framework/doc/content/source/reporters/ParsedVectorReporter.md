# ParsedVectorReporter

!syntax description /Reporters/ParsedVectorReporter

## Overview

`ParsedVectorReporter` operates on the elements contained in series of vector reporters using a [parsed function expression](MooseParsedFunction.md) as shown in [vectorMath].  This reporter allows several vector reporters to be combined into single vector.  This reporter operates on multiple vectors of the same size and returns a vector of that size.  There must be one [!param](/Reporters/ParsedVectorReporter/reporter_names) for each [!param](/Reporters/ParsedVectorReporter/reporter_symbols).
This is a vector version of the [ParsedScalarReporter.md] reporter.

!listing modules/optimization/test/tests/reporters/vector_math/vectorMath.i id=vectorMath
block=Reporters/vecs Reporters/vectorOperation

!syntax parameters /Reporters/ParsedVectorReporter

!syntax inputs /Reporters/ParsedVectorReporter

!syntax children /Reporters/ParsedVectorReporter
