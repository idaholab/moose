# RZSymmetry

This interface class provides correct behavior for RZ-symmetric problems used by THM objects.
The reason why this class is needed is because MOOSE does not support RZ-symmetric problems with arbitrarily oriented axis of symmetry.
Note that because of that the coordinate system set by THM must be Cartesian (`Moose::XYZ`) to avoid the double coordinate transformation.
Using any other coordinate system with any object using this interface is invalid.
