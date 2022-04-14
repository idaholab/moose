# Install Stochastic Tools Module with libtorch support

To be able to use the stochastic tools module with libtorch, one needs to compile
with the proper dependencies. For instuctions, see: [installation/install_libtorch.md]

## Testing the installation

Basic neural network trainer and surrogate classes have been implemented in the stochastic tools
module for testing purposes. For their documentation, see
[surrogates/LibtorchSimpleNNTrainer.md] and [LibtorchSimpleNNTrainer.md].
If MOOSE compiled without error messages, we can
navigate to the testing folder within the stochastic tools root
and use the following command to test the implementation:

```bash
cd test/tests/surrogates/libtorch_nn/
../../../../run_tests -j 8
```

The expected output should look like this:

```bash
test:surrogates/libtorch_nn.train_and_evaluate ............................................................ OK
test:surrogates/libtorch_nn.train ......................................................................... OK
test:surrogates/libtorch_nn.evaluate ...................................................................... OK
test:surrogates/libtorch_nn.retrain ....................................................................... OK
...
...
```
