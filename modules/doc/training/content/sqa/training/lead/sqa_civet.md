# CIVET Recipes for SQA


Necessary [!ac](CIVET) recipes for [!ac](SQA) including building
the documentation, running reports, and enabling code coverage.

!---

## Documentation and SQA Reports

The following blocks should be added to a recipe that
includes the following to ensure that all documentation and [!ac](SQA) checks execute on
on "Merge/Pull Requests" and the development branch.

- `trigger_pull_request = True`
- `trigger_push_branch = devel`

!listing! style=max-height:150px;
[Documentation: SQA]
script = scripts/run_cmd.sh
abort_on_failure = False
allowed_to_fail = False
APP_SUBDIR = doc
RUN_CMD = ./moosedocs.py check

[Documentation: build]
script = scripts/moosedocs.sh
abort_on_failure = False
allowed_to_fail = False
MOOSEDOCS_ARGS = --with-sqa

[Documentation: sync]
script = scripts/copy_to_remote.sh
abort_on_failure = False
allowed_to_fail = False
FROM_DIRECTORY = site
APP_SUBDIR = doc
CIVET_SERVER_POST_COMMENT=1
CHECK_EXISTS = site/index.html
CIVET_SERVER_POST_EDIT_EXISTING = 1
!listing-end!

!---

## Code Coverage

Code coverage can be added by building the the application with coverage enabled, initializing
the coverage, running desired tests, and reporting the coverage results.

!listing! style=max-height:120px;
[build]
script = scripts/build.sh
abort_on_failure = True
COVERAGE = 1

[coverage: initialize]
script = scripts/coverage_initialize.sh
abort_on_failure = False

# Add testing steps here

[coverage: report]
script = scripts/coverage_extract_and_store.sh
COVERAGE_TAG = main
!listing-end!


!alert note
The MOOSE website includes additional information regarding
[code coverage](application_development/coverage.md optional=True). Please use the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions) for assistance.
