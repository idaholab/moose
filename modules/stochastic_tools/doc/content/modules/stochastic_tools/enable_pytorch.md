# Install Stochastic Tools Module with libtorch support

To be able to use the stochastic tools module with `libtorch`, one needs to compile
with the proper dependencies. For instructions see the installation guide on the
website.

## Testing the installation

Basic artificial neural network trainer and surrogate classes have been implemented in the stochastic tools
module for testing purposes. For their documentation, see
[surrogates/LibtorchANNTrainer.md] and [surrogates/LibtorchANNSurrogate.md].
If MOOSE compiled without error messages, we can
navigate to the testing folder within the stochastic tools root
and use the following command to test the implementation (in `modules/stochastic_tools`):

```bash
./run_tests -j 8 --re libtorch_nn
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
