## Build and Test MOOSE

```bash
cd ~/projects/moose/test
make -j 6
./run_tests -j 6
```

If the installation was successful you should see most of the tests passing (some tests will be
skipped depending on your system environment).
