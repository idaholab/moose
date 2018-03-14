# MOOSE Test System

MOOSE comes with a rich system for testing MOOSE-based applications.

## Testing Philosophy

The ideas behind software testing have been constantly evolving since the beginning.  When software was small and simple you could possibly prove it to be correct.  Later on human testers were employed to find flaws in software before releases.  In the late 90s and early 00s the idea of "Extreme Programming" and "Test Driven Development" really pushed for automated testing.  Lately, automated testing was utilized in Continuous Integration and Continuous Deployment systems to allow developers to constantly integrate changes while still being able to ship a working product in a timely manner.  And most recently, sites like GitHub have allowed for every change to be tested _before_ integration within "Pull Requests".

Within MOOSE there are three different testing ideas:

1.  The "tests": which are typically "Regression Tests" consisting of input files and known good outputs ("gold" files).
2.  Unit tests that test the funcationality of small seperable pieces
3.  The TestHarness: a piece of software that was written to _run_ tests and aggregate the results.

MOOSE uses tests to do both continuous integration (CI) and continuous deployment (CD).  Each and every change to MOOSE is tested across multiple operating systems, in parallel, with threads, in debug, with Valgrind and several other configurations.  On average, our testing clusters are running ~5 Million tests every week as we develop the Framework and applications

## TestHarness

The `TestHarness` is a piece of Python software that is responsible for finding and running tests.  You can read more about it on the [TestHarness](TestHarness.md) page.