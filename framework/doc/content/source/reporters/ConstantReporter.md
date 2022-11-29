# ConstantReporter

!syntax description /Reporters/ConstantReporter

## Overview

ConstantReporter gives the ability to produce integer, real number, and string reporter data along with vectors of each type. The user must specify the following parameters for each type:

- `int`: [!param](/Reporters/ConstantReporter/integer_names) and [!param](/Reporters/ConstantReporter/integer_values)
- `Real`: [!param](/Reporters/ConstantReporter/real_names) and [!param](/Reporters/ConstantReporter/real_values)
- `std::string`: [!param](/Reporters/ConstantReporter/string_names) and [!param](/Reporters/ConstantReporter/string_values)
- `std::vector<int>`: [!param](/Reporters/ConstantReporter/integer_vector_names) and [!param](/Reporters/ConstantReporter/integer_vector_values)
- `std::vector<Real>`: [!param](/Reporters/ConstantReporter/real_vector_names) and [!param](/Reporters/ConstantReporter/real_vector_values)
- `std::vector<std::string>`: [!param](/Reporters/ConstantReporter/string_vector_names) and [!param](/Reporters/ConstantReporter/string_vector_values)

The user may specify more than one value of the same type, which will create multiple reporter values of that type.

!alert note title=Reporter names
The reporter names created by the `ConstantReporter` match the names passed in each of the `_names` parameters.

## Example Input File Syntax

!listing constant_reporter/constant_reporter.i block=constant
  indent=2 header=[Reporters] footer=[]

!syntax parameters /Reporters/ConstantReporter

!syntax inputs /Reporters/ConstantReporter

!syntax children /Reporters/ConstantReporter
