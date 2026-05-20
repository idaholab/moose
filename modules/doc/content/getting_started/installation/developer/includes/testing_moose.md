In this example, we will test the MOOSE framework. You should run this example if you are using this development environment to develop MOOSE itself. Here, we assume that a MOOSE repository is available within `~/projects/moose`. If you have cloned MOOSE in a different location, you should replace the directory in the first command that follows with the directory in which MOOSE is cloned.

```bash
cd ~/projects/moose
cd test
make -j 4
./run_tests -j 4
```

The above will compile the MOOSE test application and run the tests.
