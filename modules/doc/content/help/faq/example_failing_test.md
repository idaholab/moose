Some tests are SKIPPED. This is normal as some tests are specific to available resources, or some
other constraint your machine does not satisfy. If you see failures, or you see `MAX FAILURES`,
thats a problem! And it needs to be addressed before continuing:

- Supply a report of the *actual* failure (scroll up a ways). For example the following snippet does
  not give the full picture  (created with `./run_tests -i always_bad`):

  ```pre
  Final Test Results:
  --------------------------------------------------------------------------------
  tests/test_harness.always_ok .................... FAILED (Application not found)
  tests/test_harness.always_bad .................................. FAILED (CODE 1)
  --------------------------------------------------------------------------------
  Ran 2 tests in 0.2 seconds. Average test time 0.0 seconds, maximum test time 0.0 seconds.
  0 passed, 0 skipped, 0 pending, 2 FAILED
  ```

  Instead, you need to scroll up and report the actual error:

  ```pre
  tests/test_harness.always_ok: Working Directory: /Users/me/projects/moose/test/tests/test_harness
  tests/test_harness.always_ok: Running command:
  tests/test_harness.always_ok:
  tests/test_harness.always_ok: ####################################################################
  tests/test_harness.always_ok: Tester failed, reason: Application not found
  tests/test_harness.always_ok:
  tests/test_harness.always_ok .................... FAILED (Application not found)
  tests/test_harness.always_bad: Working Directory: /Users/me/projects/moose/test/tests/test_harness
  tests/test_harness.always_bad: Running command: false
  tests/test_harness.always_bad:
  tests/test_harness.always_bad: ###################################################################
  tests/test_harness.always_bad: Tester failed, reason: CODE 1
  tests/test_harness.always_bad:
  tests/test_harness.always_bad .................................. FAILED (CODE 1)
  ```
