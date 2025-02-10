mpirun -np 10 ./stochastic_tools-opt --allow-test-objects -i
parallel_storage_main.i

then

sub.i and test_sub.i can be run individually. Results should match. Change
parameters on both "S" and "D" and test for different values.
