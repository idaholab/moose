# EulerAngleUpdateFromReporter

The `EulerAngleUpdateFromReporter` user object is a `EulerAngleProvider` that is derived from [EulerAngleFileReader](EulerAngleFileReader.md), inheriting its ability to read initial grain orientations for each material block from a file. In addition to this functionality, the `EulerAngleUpdateFromReporter` object also updates the Euler angles for each grain (material block) using values reported from a separate source. The current implementation overwrites grain orientation information from previous steps with values from the reporter. Users should ensure the correctness of the assigned values in the reporter to maintain simulation accuracy.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/block_orientation/euler_angle_from_reporter.i block=UserObjects/euler_angle_file

!syntax parameters /UserObjects/EulerAngleUpdateFromReporter

!syntax inputs /UserObjects/EulerAngleUpdateFromReporter

!syntax children /UserObjects/EulerAngleUpdateFromReporter
