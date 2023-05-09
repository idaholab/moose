## Failing Tests id=failingtests

If many, or all tests are failing, it is a good chance the fix is simple. Follow through these steps
to narrow down the possible cause.

First, run a test that should always pass:

```bash
cd moose/test
make -j 8
./run_tests -i always_ok -p 2
```

!alert note title=did make -j 8 fail?
If `make -j 8` fails, please proceed to [Build Issues](help/troubleshooting.md#buildissues) above.
This is most likely why all your tests are failing.

This test, proves the TestHarness is available. That libMesh is built, and the TestHarness has a
working MOOSE framework available to it. Meaning, your test that is failing may be beyond the scope
of this troubleshooting guide. However, do continue to read through the bolded situations below. If
the error is not listed, please submit your failed test results to the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions) for help.

!include faq/example_failing_test.md

If the test did fail, chances are your test and our test is failing for the same reason:

- +Environment Variables+ is somehow instructing the TestHarness to use improper paths. Try each of
  the following and re-run your test again. You may find you receive a different error each time.
  Simply continue troubleshooting using that new error, and work your way down. If the error is not
  listed here, then it is time to ask the
  [MOOSE Discussion forum](https://github.com/idaholab/moose/discussions) for help:

  - check if `echo $METHOD` returns anything. If it does, try unsetting it with `unset METHOD`
  - check if `echo $MOOSE_DIR` returns anything. If it does, try unsetting it with `unset MOOSE_DIR`
  - check if `echo $PYTHONPATH` returns anything. If it does, try unsetting it with `unset PYTHONPATH`

    !alert note title=METHOD and MOOSE_DIR
    If these were set, it will be necessary to perform a rebuild. See
    [Build Issues](help/troubleshooting.md#buildissues) above.

- +Failed to import hit+:

  - Verify you have activated the conda moose environment: `echo $CONDA_DEFAULT_ENV`. This command
    should return 'moose'. If not, see [Conda Issues](help/troubleshooting.md#condaissues) above.

- +Application not found+

  - Your Application has not yet been built. You need to successfully perform a `make`. If make is
    failing, please see [Build Issues](help/troubleshooting.md#buildissues) above.

  - Perhaps you have specified invalid arguments to run_tests? See
    [TestHarness More Options](TestHarness.md#moreoptions). Specifically for help with:

    - `--opt`
    - `--dbg`
    - `--oprof`

- +gethostbyname failed, localhost (errno 3)+

  - This is a fairly common occurrence which happens when your internal network stack / route, is
    not correctly configured for the local loopback device. Thankfully, there is an easy fix:

    - Obtain your hostname:

      ```bash
      hostname

      mycoolname
      ```

    - Linux & Macintosh : Add the results of `hostname` to your `/etc/hosts` file. Like so:

      ```bash
      sudo vi /etc/hosts

      127.0.0.1  localhost

      # The following lines are desirable for IPv6 capable hosts
      ::1        localhost ip6-localhost ip6-loopback
      ff02::1    ip6-allnodes
      ff02::2    ip6-allrouters

      127.0.0.1  mycoolname  # <--- add this line to the end of your hosts file
      ```

      Everyones host file is different. But the results of adding the necessary line described above
      will be the same.

    - Macintosh only, 2nd method:

      ```bash
      sudo scutil --set HostName mycoolname
      ```

      We have received reports where the second method sometimes does not work.

- +TIMEOUT+

  - If your tests fail due to timeout errors, it is most likely you have a good installation, but a
    slow machine (or slow filesystem). You can adjust the amount of time that the TestHarness allows
    a test to run before timing out, by adding a paramater to your test file:

    ```pre
    [Tests]
      [./timeout]
        type = RunApp
        input = my_input_file.i
        max_time = 300   <-- time in seconds before a timeout occurs. 300 is the default for all tests.
      [../]
    []
    ```

- +CRASH+

  - A crash indicates the TestHarness executed your application (correctly), but then your
    application exited with a non-zero return code. See
    [Build Issues](help/troubleshooting.md#buildissues) above for a possible solution.

- +EXODIFF+

  - An exodiff indicates the TestHarness executed your application, and your application exited
    correctly. However, the generated results differs from the supplied gold file. If this test
    passes on some machines, and fails on others, this would indicate you may have applied too tight
    a tolerance to the acceptable error values *for that specific machine*. We call this phenomena
    *machine noise*.

- +CSVDIFF+

  - A different file format following the same error checking paradigm as an exodiff test.
