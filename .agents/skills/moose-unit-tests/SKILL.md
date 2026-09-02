---
name: moose-unit-tests
description: >-
  Write gtest-based unit tests for MOOSE C++ code. Use when asked to add,
  extend, or find a unit test for framework/ or modules/<name>/ code - as
  opposed to a regression/integration test (an input file under a test/tests
  directory). Covers where the test file goes, which fixture base class to
  use, naming conventions, and how new files get picked up by the build.
  Building and running the resulting binary is out of scope - hand off to
  moose-verify-changes.
---

# Writing MOOSE Unit Tests

Unit tests are gtest-based and live in a dedicated `unit/` app, not the main
`test/tests` regression suite. The distinction is not whether a simulation or
an input file is involved - unit tests can and do run full `MooseApp`s,
`Executioner`s, or `.i` input files via API (`Moose::createMooseApp(...)`, as
in `MooseMainTest.C`, `ExecutorTests.C`, or `RestartAdaptivityBackupTest.C`;
supporting `.i` files for these live under `unit/files/<TestName>/`). The real
distinction is who drives it and how the result is checked: a unit test is a
gtest binary that drives everything through direct C++/API calls and asserts
with `EXPECT_*`/`ASSERT_*`; a `test/tests` test is driven externally by the
TestHarness against a `tests` spec file, running the MOOSE executable as a
subprocess and checking its output (exodiff, CSV, expected error text, exit
code, etc.) with no C++ assertions involved. Prefer a unit test whenever the
check is more naturally expressed as a C++ assertion on an API return value
than as a spec-file check on process output.

This skill only covers writing/placing the test. It does not build or run
anything - that is `moose-verify-changes`'s job, which also owns the conda
environment gate.

## Step 1 - Find the unit directory

- Code under `framework/` -> tests go in `unit/` at the repository root
  (`unit/src`, `unit/include`).
- Code under `modules/<name>/` -> tests go in `modules/<name>/unit/`
  (`modules/<name>/unit/src`, `modules/<name>/unit/include`).

There is exactly one `unit/` per app; it is not mirrored per source
subdirectory. Some `unit/src` trees keep a matching subdirectory (e.g.
`unit/src/base`) for files tied to `.../src/base`, but most files sit flat in
`unit/src` - check whether the target unit directory already has
subdirectories before deciding.

Confirm the module actually has a `unit/` directory (`ls modules/<name>/unit`).
If it does not exist yet, that is a bigger step than adding one test file -
tell the user rather than scaffolding a new unit app.

## Step 2 - Look for an existing test file first

Before creating a new file, check whether a `<ClassName>Test.C` (or
`.../unit/src/**/<ClassName>Test.C`) already exists for the class/utility
under test - new cases usually belong there as additional `TEST`/`TEST_F`
bodies, not a new file. Use CodeGraph or:

```bash
find unit modules/<name>/unit -iname "*<ClassName>*Test*" 2>/dev/null
```

## Step 3 - Pick the right test shape

**Plain function/utility with no MOOSE object dependencies** - a free
`TEST(Suite, Case)`, no fixture:

```cpp
TEST(FrictionProjection, StickAndSlipRoots)
{
  const std::array<Real, 1> stick_pressure = {1.0};
  ...
  EXPECT_DOUBLE_EQ(ac_stick[0], 0.0);
}
```

`Suite` is normally the name of the class/namespace under test (no `Test`
suffix needed there - the file name carries that). Group related cases
under the same suite name so `--gtest_filter=Suite.*` selects them together.

**Testing a MOOSE object that can be built standalone (UserObject, Function,
etc., without a mesh-dependent setup)** - inherit `MooseObjectUnitTest`
(`framework/include/base/MooseObjectUnitTest.h`), which builds a minimal mesh
+ `FEProblem` for you:

```cpp
class MyUnitTest : public MooseObjectUnitTest
{
public:
  MyUnitTest() : MooseObjectUnitTest("MyAppUnitApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters params = _factory.getValidParams("MyObjectThatIAmTesting");
    _obj = &addObject<MyObjectThatIAmTesting>("MyObjectThatIAmTesting", "name", params);
  }

  const MyObjectThatIAmTesting * _obj;
};

TEST_F(MyUnitTest, doesTheThing)
{
  EXPECT_EQ(_obj->method(), expected);
}
```

This fixture's mesh/`FEProblem` is not wired up for a real solve, so it does
not work for Kernels, BCs, or anything that needs to be assembled against a
real mesh/system. Testing those means either driving a full, solvable
`FEProblem`/`Executioner` via API from a heavier unit test (see the
`ExecutorTests.C`-style approach above) or falling back to a `test/tests`
input-file test - use whichever is more natural for the assertions you need
to make.

**Anything needing custom per-test setup/teardown** (temp files, saved
global state, etc.) - a plain `::testing::Test` fixture with `SetUp()` /
`TearDown()` overrides, as in `unit/src/DataFileUtilsTest.C`.

If in doubt which shape fits, look at an existing test for a sibling class
(same base class, same module) and match its structure.

## Step 4 - File conventions

- File name: `<ClassOrConceptName>Test.C` in `.../unit/src` (or the matching
  subdirectory if one already exists for that source area). A fixture class
  declared separately goes in the matching `.../unit/include/<Name>Test.h`
  header - most tests keep the fixture in the `.C` file instead and only need
  a header when it is shared across multiple `.C` files.
- Start with the standard MOOSE license header (copy it verbatim from a
  neighboring file in the same unit directory).
- Include `"gtest/gtest.h"` (or `"gtest_include.h"` in framework `unit/`,
  which wraps it with libMesh's warning-suppression pragmas) before other
  headers.
- No new file needs to be registered anywhere: both `unit/Makefile` and
  `modules/<name>/unit/Makefile` glob `find .../unit/src -name "*.C"`, so any
  `.C` file placed under `unit/src` is picked up automatically. A new
  `.../unit/include` header needs no registration either.
- If a test needs a supporting `.i` input file (loaded via
  `Moose::createMooseApp(...)`, not run through the TestHarness), put it under
  `.../unit/files/<TestName>/` and reference it with a path relative to the
  unit binary's working directory (e.g. `files/<TestName>/foo.i`), matching
  `MooseMainTest.C`.
- Follow the repository's normal C++ style (`AGENTS.md`): `make_range` for
  index loops, `libmesh_map_find` for map lookups, comments only where the
  code doesn't speak for itself, doxygen on any fixture class whose purpose
  isn't obvious from its name.

## Step 5 - Handoff

Once the test file is written, verifying it compiles and passes is
`moose-verify-changes`'s job (it owns the conda/environment gate and the
build step) - invoke that skill rather than building or running the unit
binary directly here.
