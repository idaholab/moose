In this example, we will test building and running tests for a MOOSE-based application. You should run this example if you are using this development environment to develop a MOOSE-based application. Here, you should replace the directory in the first command that follows (`/path/to/app`) to the directory that your MOOSE-based application is in.

```bash
cd /path/to/app
make -j 4
./run_tests -j 4
```

The above will compile the MOOSE-based application and run its tests.
