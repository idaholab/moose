# ParsedVectorReporter

!syntax description /Reporters/ParsedVectorReporter

## Overview

`ParsedVectorReporter` operates on the elements contained in series of vector reporters using a [parsed function expression](MooseParsedFunction.md) as shown in [transientReporterTest].  This reporter allows several vector reporters to be combined into a single vector.  This reporter operates on multiple vectors of the same size and returns a vector of that size.  There must be one [!param](/Reporters/ParsedVectorReporter/vector_reporter_names) for each [!param](/Reporters/ParsedVectorReporter/vector_reporter_symbols).  This reporter can also use postprocessor values in the parsed function using [!param](/Reporters/ParsedVectorReporter/scalar_reporter_names) and [!param](/Reporters/ParsedVectorReporter/scalar_reporter_symbols).  As shown in [transientReporterTest], this allows the timestep size given by [TimestepSize.md] to be included in the parsed function.
This is a vector version of the [ParsedScalarReporter.md] reporter.

!listing parsed_reporters/transientParsedVec.i id=transientReporterTest block=Reporters/vectorOperation

!syntax parameters /Reporters/ParsedVectorReporter

!syntax inputs /Reporters/ParsedVectorReporter

!syntax children /Reporters/ParsedVectorReporter
