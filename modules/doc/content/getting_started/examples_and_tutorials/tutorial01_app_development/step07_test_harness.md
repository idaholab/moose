# Step 7: Develop an Application Test

!alert construction
The remainder of this tutorial is currently being developed. More content should be available soon. For now, refer back to the [examples_and_tutorials/index.md] page for other helpful training materials or check out the MOOSE [application_development/index.md] pages for more information.

<!--use the same mesh as simple diffusion-->

<!--turn pressure_diffusion.i into a test. Explain how to run in parallel and demonstrate. Introduce postprocessors to output results and explain how postprocs will be the subject of a subsequent step. We already know this solution to be good and true, so its a good test of our DarcyPressyre object.-->

<!--show all test types: csvdiff, exodiff, and runexception. The expected error could be from the range check for the viscosity \ne 0, although this test isn't really necessary since it is not an object of the DarcyPressure class - but its fine, just do it anyways and explain why its not totally necessary-->

!content pagination previous=tutorial01_app_development/step06_input_params.md
