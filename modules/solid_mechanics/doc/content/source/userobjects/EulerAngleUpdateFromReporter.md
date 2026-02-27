# EulerAngleUpdateFromReporter

The `EulerAngleUpdateFromReporter` user object is a `EulerAngleProvider` that is derived from [EulerAngleFileReader](EulerAngleFileReader.md), inheriting its ability to read initial grain orientations for each material block from a file. In addition to this functionality, the `EulerAngleUpdateFromReporter` object also updates the Euler angles for each grain (material block) using values reported from a separate source. The current implementation overwrites grain orientation information from previous steps with values from the reporter. Users should ensure the correctness of the assigned values in the reporter to maintain simulation accuracy.

## Example Input File Syntax

The `EulerAngleUpdateFromReporter` object is designed to be used in tandem with a reporter, as its name implies. This reporter should supply the necessary Euler angles information for each grain (or subdomain). The reporter's output should consist of four columns:

1. Subdomain ID (Grain ID): This column should contain unique identifiers for each grain.
2. Euler Angle 1: This column should provide the first component of the Euler angles (in degrees) for the corresponding grain.
3. Euler Angle 2: This column should provide the second component of the Euler angles (in degrees) for the corresponding grain, in degrees.
4. Euler Angle 3: This column should provide the third component of the Euler angles (in degrees) for the corresponding grain, in degrees.

Following is an example of such a reporter named `updated_ea` with column names `ea0 ea1 ea2` and `subdomain_id`:

!listing modules/solid_mechanics/test/tests/block_orientation/euler_angle_from_reporter.i block=Reporters/updated_ea

Then, in the definition of the `EulerAngleUpdateFromReporter` object, one should fill the input options, `grain_id_name` and Euler angle components, i.e., `euler_angle_0_name`, `euler_angle_1_name`, `euler_angle_2_name` with the corresponding `<reporter name>/<column name>` pairs. For example, in this specific case, the `reporter name` is `updated_ea`, and the `column name` are listed  in the `real_vector_names` in the `Reporters` section.

!listing modules/solid_mechanics/test/tests/block_orientation/euler_angle_from_reporter.i block=UserObjects/euler_angle_file

!syntax parameters /UserObjects/EulerAngleUpdateFromReporter

!syntax inputs /UserObjects/EulerAngleUpdateFromReporter

!syntax children /UserObjects/EulerAngleUpdateFromReporter
