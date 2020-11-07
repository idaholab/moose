# ConstantReporter

!syntax description /Reporters/ConstantReporter

## Overview

ConstantReporter gives the ability to produce integer, real number, vector, and string reporter data. The user must specify [!param](/Reporters/ConstantReporter/value_types) to let the system know what kind of data is being constructed:

- `integer`: int
- `real`: Real or double
- `vector`: std::vector<Real> or std::vector<double>
- `string`: std::string

The user may specify more than one value of the same type, which will create multiple reporter values of that type.

For each [!param](/Reporters/ConstantReporter/value_types) specified, the user must specify a name for it with [!param](/Reporters/ConstantReporter/names).

For each [!param](/Reporters/ConstantReporter/value_types) specified, there must also be a value associated with it using [!param](/Reporters/ConstantReporter/integer_values), [!param](/Reporters/ConstantReporter/real_values), [!param](/Reporters/ConstantReporter/vector_values), and/or [!param](/Reporters/ConstantReporter/string_values).

## Example Input File Syntax

This input is specifying a constant reporter with three integer values, two real number values, two vectors, and one string:

!listing constant_reporter/constant_reporter.i block=constant
  indent=2 header=[Reporters] footer=[]

!syntax parameters /Reporters/ConstantReporter

!syntax inputs /Reporters/ConstantReporter

!syntax children /Reporters/ConstantReporter
