# SQA Setup for MOOSE-based Application

!---

## MooseDocs

[MooseDocs](MooseDocs/index.md optional=True) is utilized to minimize the effort necessary for a
MOOSE-based application

These instructions assume that you have documentation setup within your application, which
should be the case for most applications.

To test run the following commands, if you have problems please use the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions) for assistance.

```
cd doc
./moosedocs.py build --serve
```

!---

## Update Content

To allow for the design documentation to be complete, the content of the [!ac](MOOSE) framework and
all modules enabled for an application must be added to the "config.yml" within
the application "doc" directory.

A basic application that uses the heat transfer module, should have a "Contents" section
in the "config.yml" should be as follows.

```
Content:
    - ${ROOT_DIR}/doc/content
    - ${MOOSE_DIR}/framework/doc/content
    - ${MOOSE_DIR}/modules/heat_conduction/doc/content
```

!---

## Update Include Directories

To correctly populate `MooseObject` inheritance lists, the list of include files should also
be updated within the MooseDocs ["appsyntax"](MooseDocs/extensions/appsyntax.md optional=True) extension
section of the "config.yml" file as follows.

```
Extensions:
    MooseDocs.extensions.appsyntax:
        executable: ${ROOT_DIR}
        includes:
            - ${ROOT_DIR}/include
            - ${MOOSE_DIR}/framework/include
            - ${MOOSE_DIR}/modules/heat_conduction/include
```

!alert note
This is not necessary for NQA-1 compliance, but it is good practice for complete documentation.

!---

## Enable Template Extension

The MooseDocs ["template"](MooseDocs/extensions/template.md optional=True) extension is leveraged
to allow for sharing of [!ac](SQA) related documentation to minimize the effort necessary
to build NQA-1 compliant software.

To enable the extension, add the following to the "config.yml" file, within the "Extensions"
section.

```
  MooseDocs.extensions.template:
    active: True
```

!---

## Create SQA Documents

PLN-4005 includes a list of "repository documentation" that are necessary to be compliant with
the [!ac](INL) [!ac](SQA) process. Templates exist for a majority of these documents.

Within a MOOSE-based application (e.g., "blackbear"), the following files should be created
with in the "doc/content/sqa" directory. The prefix should be a reflection of the application name.

- blackbear_cci.md, blackbear_far.md, blackbear_rtm.md, blackbear_scs.md, blackbear_sdd.md,
  blackbear_sll.md, blackbear_srs.md, blackbear_stp.md, blackbear_vvr.md

!alert note
The "blackbear_vvr.md" is an optional file. This is the Verification and Validation Report (VVR) that
can be used to detail verification and validation tests (in the scientific sense) for
an application.

!---

## SQA Template Content

Within each of the template files, load the corresponding "app" template file. For example, within
the "blackbear_sdd.md" file the following should be included.

```
!template load file=app_sdd.md.template app=BlackBear category=blackbear
```

The "category" should match the prefix of the file names and the "app" be the registered MOOSE-based
application name.

!alert note
Certain templates have required sections and others have optional sections.
The required sections will be detailed upon rendering the website with the "build" command.

!---

## SQA Reports

To aid in monitoring the status of [!ac](SQA) documentation a command-line reporting system exists.
To use this system a report configuration file ("sqa_reports.yml") must be created in the "doc"
directory of the MOOSE-based application.

This file should contain three sections: "Applications", "Documents", and "Requirements". In the
following slides the configuration from the "blackbear" application is presented. The file
for any application will be nearly identical, with the application name being the major difference.

!---

### SQA Reports: Applications

The application reports provide feedback regarding the existence of the application
design pages, which are associated with the registered objects.

```
Applications:
    blackbear:
        exe_directory: ${ROOT_DIR}
        exe_name: blackbear
        app_types:
            - BlackbearApp
        content_directory: doc/content
```

!---

### SQA Reports: Documents

The document reports provide assurance that the necessary [!ac](SQA) documents are in place
for conformance with PLN-4005.

!listing style=max-height:200px;
Documents:
    working_dirs:
        - ${BLACKBEAR_DIR}/doc/content
        - ${MOOSE_DIR}/framework/doc/content
    software_requirements_specification: sqa/blackbear_srs.md
    software_design_description: sqa/blackbear_sdd.md
    software_test_plan: sqa/blackbear_stp.md
    requirements_traceability_matrix: sqa/blackbear_rtm.md
    verification_validation_report: sqa/blackbear_vvr.md
    failure_analysis_report: sqa/blackbear_far.md
    safety_software_determination: sqa/inl_records.md#safety_software_determination
    quality_level_determination: sqa/inl_records.md#quality_level_determination
    enterprise_architecture_entry: sqa/inl_records.md#enterprise_architecture_entry
    software_quality_plan: sqa/inl_records.md#software_quality_plan
    configuration_management_plan: sqa/inl_records.md#configuration_management_plan
    asset_management_plan: sqa/inl_records.md#asset_management_plan
    verification_validation_plan: sqa/inl_records.md#verification_and_validation_plan
    software_library_list: sqa/blackbear_sll.md
    communication_and_contact_information: sqa/blackbear_cci.md
    software_coding_standards: sqa/blackbear_scs.md
    user_manual: syntax/blackbear_index.md   # symlink to avoid duplicate file errors
    theory_manual: syntax/blackbear_index.md
    show_warning: false


!---

### SQA Reports: Requirements

The document reports provide assurance that the test specifications are properly
documented with a requirement, design, and issues as described in PLN-4005.


```
Requirements:
    blackbear:
        directories:
            - test/tests
        log_testable: WARNING
        show_warning: false
        include_non_testable: true
```

Multiple test specifications files can be added (such as when `tests` and `assessments` are used as
filenames for organization) by setting the `specs:` parameter:

```
Requirements:
    blackbear:
        directories:
            - test/tests
        specs:
            - tests
            - assessments
        log_testable: WARNING
        show_warning: false
        include_non_testable: true
```

!---

### SQA Reports: Running

After building an "sqa_reports.yml" configuration file for an application, the reports can be
produced by running the MooseDocs executable with the "check" command.

```
cd docs
./moosedocs.py check
```

!---

## SQA Extension

The [!ac](SQA) documentation and report information is added to the "config.yml" using the
MooseDocs ["sqa"](MooseDocs/extensions/sqa.md optional=True) extension. This is accomplished by
adding the extension to the "Extensions" section. For example, the following is what should be
included for an application that includes the heat transfer module.

```
    MooseDocs.extensions.sqa:
        active: True
        reports: !include ${ROOT_DIR}/doc/sqa_reports.yml
        repos:
            default: https://github.com/idaholab/blackbear
            moose: https://github.com/idaholab/moose
        categories:
            framework: !include ${MOOSE_DIR}/framework/doc/sqa_framework.yml
            heat_conduction: !include ${MOOSE_DIR}/modules/heat_conduction/doc/sqa_heat_conduction.yml
            blackbear: !include ${ROOT_DIR}/doc/sqa_blackbear.yml
```

!---

### SQA Extension: sqa_blackbear.md

The above configuration includes an "sqa_blackbear.yml" file. It provides a connection between the
report configuration and the MooseDocs website configuration. This is done in a separate file
to allow for applications to share this information.

The included file simply indicates where to locate the tests for the application: in the "tests" spec
with the "test/tests" directories.

```
specs:
    - tests
directories:
    - ${BLACKBEAR_DIR}/test/tests
```

!---

## CIVET Extension

The testing results from [!ac](CIVET) can be displayed by enabling the
MooseDocs ["civet"](MooseDocs/extensions/sqa.md optional=True) extension within
the "Extensions" sections of the "config.yml" file. The following is an example from the "blackbear"
application.


```
    MooseDocs.extensions.civet:
        test_results_cache: '/tmp/civet/jobs'
        remotes:
            blackbear:
                url: https://civet.inl.gov
                repo: idaholab/blackbear
            moose:
                url: https://civet.inl.gov
                repo: idaholab/moose
                location: ${MOOSE_DIR}
```

!---

## SQA Documentation

To build the complete documentation, including the [!ac](SQA) pages, the following method is
required when running the build command.

```
cd doc
./moosedocs.py build --with-sqa --serve
```
