# AccumulateReporter

!syntax description /Reporters/AccumulateReporter

## Overview

AccumulateReporter gives the ability to accumulate reporter values over time and assigns it to a vector reporter value. Currently supported reporter value types:

- `int`
- `Real` (this covers [Postprocessors/index.md])
- `std::string`
- `std::vector<int>`
- `std::vector<Real>` (this covers [VectorPostprocessors/index.md])
- `std::vector<std::string>`

If more types need to be supported, they can easily be added by adding a line in the `if` statement in `AccumulateReporter::initialSetup()`:

!listing framework/src/reporters/AccumulateReporter.C
         re=void\nAccumulateReporter::initialSetup.*?^}

!alert note title=Reporter names
The reporter names created by the `AccumulateReporter` match each of the accumulated reporters' names.

!syntax parameters /Reporters/AccumulateReporter

!syntax inputs /Reporters/AccumulateReporter

!syntax children /Reporters/AccumulateReporter
