#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from dataclasses import dataclass
from typing import Any, Optional, Tuple, Union
import json

from numpy import float64
from numpy.typing import NDArray

# The valid numeric data types
ValidationNumericDataType = Union[float, list[float]]
# Valid numeric vector types
ValidationNumericVectorType = Union[NDArray[float64], list[float]]
# Input type for addVectorData
ValidationVectorDataInputType = Tuple[ValidationNumericVectorType, str, Optional[str]]

@dataclass(kw_only=True)
class ValidationResult:
    """
    Data structure that stores the information
    about a single validation result.
    """
    # The status
    status: None
    # A human readable message
    message: str
    # The test method the result came from
    test: str
    # Whether or not this result is considered
    # a validation result (enables running verification
    # cases but not storing them in a database)
    validation: bool = True
    # The data that this result is attached to,
    # if any
    data_key: Optional[str] = None

@dataclass(kw_only=True)
class ValidationData:
    """
    Base data structure that stores the information
    about a piece of validation data to be stored
    """
    def __post_init__(self):
        assert isinstance(self.key, str)
        assert isinstance(self.description, str)
        assert isinstance(self.test, (str, type(None)))
        assert isinstance(self.validation, bool)

        try:
            json.dumps(self.value)
        except (TypeError, OverflowError) as e:
            raise TypeError(f'Data type "{type(self.value)}" is not JSON serializable') from e

    # The data key
    key: str
    # The data
    value: Any
    # Human readable description of the data
    description: str
    # The test that added this data, if any
    test: Optional[str]
    # Whether or not this result is considered
    # a validation result (enables running verification
    # cases but not storing them in a database)
    validation: bool = True

@dataclass(kw_only=True)
class ValidationNumericData(ValidationData):
    """
    Data structure that stores the information about
    a piece of numeric validation data that can be checked
    """
    def __post_init__(self):
        super().__post_init__()
        assert isinstance(self.units, (str, type(None)))
        if self.nominal is not None:
            assert isinstance(self.nominal, type(self.value))
        if self.bounds is not None:
            assert isinstance(self.bounds, tuple)
            assert len(self.bounds) == 2
            assert isinstance(self.bounds[0], type(self.value))
            assert isinstance(self.bounds[1], type(self.value))
        if self.rel_err is not None:
            assert isinstance(self.rel_err, float)

    # Units for the data, if any
    units: Optional[str]
    # A nominal value for this data; unused
    # in the test but useful in postprocessing
    nominal: Optional[ValidationNumericDataType] = None
    # Bounds for the data (min and max)
    bounds: Optional[Tuple[ValidationNumericDataType, ValidationNumericDataType]] = None
    # Allowed relative error for the data
    rel_err: Optional[float] = None

@dataclass(kw_only=True)
class ValidationScalarData(ValidationNumericData):
    """
    Data structure that stores the information about
    a piece of scalar numeric data that can be checked
    """

@dataclass(kw_only=True)
class ValidationVectorData(ValidationNumericData):
    """
    Data structure that stores the information about
    a piece of array numeric validation data that can be checked
    """
    def __post_init__(self):
        super().__post_init__()
        if self.nominal is not None:
            assert len(self.value) == len(self.nominal)
        if self.bounds is not None:
            assert len(self.value) == len(self.bounds[0])
            assert len(self.value) == len(self.bounds[1])
        assert isinstance(self.x, list)
        assert isinstance(self.x, type(self.value))
        assert len(self.x) == len(self.value)
        assert isinstance(self.description, str)
        assert isinstance(self.x_units, (str, type(None)))

    # The x values for the data
    x: list[float]
    # The description for the x data
    x_description: str
    # Units for the x data, if any
    x_units: Optional[str]

# The valid data types that can be stored
ValidationDataTypes = [ValidationData, ValidationScalarData, ValidationVectorData]
# The valid data types that can be stored, in string form
ValidationDataTypesStr: list[str] = [v.__name__ for v in ValidationDataTypes]
