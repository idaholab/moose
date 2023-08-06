# ADShaftConnectedMotorUserObject

!syntax description /UserObjects/ADShaftConnectedMotorUserObject

The torque and moment of inertia are specified as [Functions](syntax/Functions/index.md),
that only depend on the shaft speed variable.

!alert note
The `time` argument of the Functions is used to evaluate the `Functions` at the shaft speed.

!alert note
This user object is created automatically by the [ShaftConnectedMotor.md]
component (if its [!param](/Components/ShaftConnectedMotor/ad) parameter is set to true),
users do not need to add it to an input file.

!syntax parameters /UserObjects/ADShaftConnectedMotorUserObject

!syntax inputs /UserObjects/ADShaftConnectedMotorUserObject

!syntax children /UserObjects/ADShaftConnectedMotorUserObject
