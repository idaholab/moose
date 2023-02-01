The scope of [!ac](MOOSE) Tools is to provide a set of utilities to support [!ac](MOOSE) development.
Namely, verification, testing, documentation, and data analysis and manipulation of [!ac](MOOSE) code
and [!ac](I/O). Tools code is written in a general way, so that it can be extended and tuned to the
needs of the user/developer. The major two systems of the Tools utilities are described in the following
sections, but more information on the support utiltities also contained within MOOSE tools can be
found on the [main documentation page](python/index.md).

### MooseDocs

The [python/MooseDocs/index.md] facilitates documentation of the [!ac](MOOSE) and [!ac](MOOSE) Tools code
bases, as well as supports the [!ac](MOOSE) [!ac](SQA) practices. It contains many extensions for website
rendering, navigation, linking, bibliographic references, and support for integration of code and input
file snippets (among many more features), as well as capabilities for development of training and
presentation slides and reports.

### TestHarness

The [TestHarness.md] system is responsible for finding tests and running them. The extended
philosophy behind MOOSE testing can be found in the [application_development/test_system.md]
documentation page,  and this philsophy has driven the creation and design choices of the TestHarness
system.

!include application_development/test_system.md start=Within MOOSE there are three end=MOOSE uses tests

The TestHarness integrates with the [!ac](MOOSE) [!ac](CI) and [!ac](CD) workflows to facilitate testing
across multiple operating systems and hardware architectures.
