# Software Quality Assurance Extension

The [!ac](SQA) extension is design to aid in tracking documentation for satisfying the NQA-1 standard for
software quality. It is simply a tool to aid MOOSE-based software projects for being sure that all
the necessary information exists to meet this standard. The first section details the use of
templates for building software quality documentation, the second explains the extension level
configuration, and the final section briefly discuss each of the available extension commands and
the intended purpose. For a working example please refer to the
[MOOSE SQA](sqa/index.md exact=True optional=True) documentation.

## SQA Templates

The SQA extension leverages the templates to aid in setting up the necessary documents to satisfy the
NQA-1 standard. For details of how to utilize the template system please refer to the
[extensions/template.md] documentation. There are two sets of templates, the general set and the
MOOSE-based application.

!alert note title=Naming of files is important
When using the MOOSE-based application documentation templates, the file names used are critical
for the system to correctly operate. See [#moose-based-application-templates] for details.

The general set is comprised of templates that implement the standard [!ac](INL) templates using
MooseDocs. This is used by the framework to define the core of the SQA documentation that MOOSE-based
applications can link against or could be used by other applications that desire to utilize
MooseDocs, but are not a traditional MOOSE-based application.

The set of MOOSE-based application templates should be the starting point for most application
developers adding SQA documentation. Using the templates will automatically link in the necessary
documents from the framework and modules with the documents from an application to make a
complete set of documents.

### MOOSE-based Application Templates id=moose-based-application-templates

The following is a complete list of templates that should be included within an application to
complete the SQA documentation. With the exception of the index template (app_index.md.template) the
names of the files that load the templates should begin with the category name and be followed by
the three letter suffix used by the template being loaded. The category name is the name provided in
the "categories" configuration of the extension. Please refer to the [#extension-configuration]
section below for details. For example, if in the configuration the category was named "bull_frog"
then the "app_stp.md.template" file should be loaded in a file named "bull_frog_stp.md".

- +app_index.md.template+\\
  This is used as the SQA landing page and will automatically link to the other SQA documents
  listed below. The name of the file loading this template is not restricted, but in general it is
  recommended that it be the index file within an "sqa" folder (e.g., "sqa/index.md").
- +app_stp.md.template+\\
  This will comprise the application Software Test Plan.
- +app_sdd.md.template+\\
  This will comprise the application Software Design Document.
- +app_srs.md.template+\\
  This will comprise the application Software Requirement Specification.
- +app_rtm.md.template+\\
  This will comprise the application Requirement Traceability Matrix.
- +app_vvr.md.template+\\
  This will comprise the application Verification and Validation Report.
- +app_far.md.template+\\
  This will comprise the application Failure Analysis Report.
- +app_sll.md.template+\\
  This will comprise the application Software Library List.
- +app_cci.md.template+\\
  This will comprise the application Communications and Contact Information.
- +app_scs.md.template+\\
  This will comprise the application Software Coding Standards.

### MOOSE Module Templates

The following is a list of templates that should be included within a MOOSE module to complete the
SQA documentation. Similar naming conventions that applied to [#moose-based-application-templates]
apply to MOOSE module templates. There are no [!ac](CCI) or [!ac](SCS) templates for modules because
they follow the framework versions of the [framework_cci.md] and [framework_scs.md] documents.

- +module_stp.md.template+\\
  This will comprise the module Software Test Plan.
- +module_sdd.md.template+\\
  This will comprise the module Software Design Document.
- +module_srs.md.template+\\
  This will comprise the module Software Requirement Specification.
- +module_rtm.md.template+\\
  This will comprise the module Requirement Traceability Matrix.
- +module_vvr.md.template+\\
  This will comprise the module Verification and Validation Report.
- +module_far.md.template+\\
  This will comprise the module Failure Analysis Report.
- +module_sll.md.template+\\
  This will comprise the module Software Library List.

### General Templates

The available general SQA templates are listed below. Unlike the MOOSE-based application or module
versions above, a landing page template does not exist and there is no naming convention required for
loading these templates. These templates based on the [!ac](INL) templates and are stand-alone to
aid in creating SQA documentation for unique non-MOOSE applications. They are also used for the
framework documents on which all MOOSE-based applications depend.

- +stp.md.template+\\
  [!ac](INL) template for the Software Test Plan.
- +sdd.md.template+\\
  [!ac](INL) template for the Software Design Document.
- +srs.md.template+\\
  [!ac](INL) template for the Software Requirement Specification.
- +rtm.md.template+\\
  [!ac](INL) template for the Requirement Traceability Matrix.
- +vvr.md.template+\\
  A template to build a Verification and Validation Report, [!ac](INL) does not supply a template.
- +far.md.template+\\
  A template to build a Failure Analysis Report, [!ac](INL) does not supply a template.
- +sll.md.template+\\
  A template to build a Software Library List, [!ac](INL) does not supply a template.
- +cci.md.template+\\
  A template to build a Communication and Contact Information, [!ac](INL) does not supply a template.
- +scs.md.template+\\
  A template to build a Software Coding Standards, [!ac](INL) does not supply a template.


## Extension Configuration

[config-sqa-ext] lists the available configuration settings for the SQA extension. These
configuration items should be included in the MooseDocs configuration file (e.g., config.yml). The
options for this extension are fairly intricate and thus each of the options shall be discussed in
detail.

!devel settings id=config-sqa-ext
                caption=Configuration options for the SQA extension.
                module=MooseDocs.extensions.sqa
                object=SQAExtension

### +`repos`+

This option is a dictionary of names to URLs that can be used when linking issues within a
test specification. By default this option is populated with a dictionary that links the "default"
keyword to the MOOSE repository. Any test specification that includes an "issues" list
will default to using the MOOSE repository for linking to that issue. As such, MOOSE-based
applications should specify this option with the correct repository as the default. For example,
[example-repo-config] sets the default URL. This example also demonstrates how to add
additional URLs (i.e., pika) to the list of available URLs.

!listing caption=Example use of "repos" configuration option. id=example-repo-config
MooseDocs.extensions.sqa:
    repos:
        default: https://github.com/idaholab/mastodon
        pika: https://github.com/idaholab/pika

Within a test specification the 'issues' can then include issue numbers from multiple repositories.
Again, using the example in [example-repo-config] a possible 'issues' list could look as follows,
where the number without a prefix would link to the URL listed in 'default' and the number with
the "pika" prefix would link to the 'pika' URL.

!listing
[Tests]
  issues = '#123 pika#123'

!alert note title=MOOSE and libMesh are always available.
The MOOSE and libMesh repositories are automatically added using the "moose" and "libmesh" keywords.
Therefore issues numbers such as `moose#1234` and  `libmesh#1234` will always be available and
do not need to be specified in the `repos` option.

### +`categories`+ id=categories-config

This option defines the various sources of SQA documentation to include in the documentation for
an application. This allows one application to include SQA documentation from another application.
The point of this feature is allow SQA documents to build on each other---in the same way that
MOOSE allows physics to build on each other---to minimize the effort needed to build complete
SQA documents.

The `categories` option is a dictionary of dictionaries that include four keys: `specs`,
`dependencies`, `repo`, and `reports`. [example-categories-config] is an example of the nested
dictionary used for this option. These sub-options are detailed as follows.

- `directories`: A list of directories to search for requirements that are defined within test
  specifications.
- `specs`: A list of test specifications names.
- `dependencies`: A list of category dependencies. For example, in [example-categories-config]
  the "heat_conduction" category depends on the "framework" category. This list allows the
  dependency lists within the various SQA documents.
- `reports`: Options for SQA report generation, see [python/moosesqa/index.md] for additional
  details.

!listing caption=Example use of "categories" configuration option. id=example-categories-config
MooseDocs.extensions.sqa:
    categories:
        framework:
            directories:
                - ${MOOSE_DIR}/test/tests
            specs:
                - tests
            repo: moose
         heat_conduction:
            directories:
                - ${MOOSE_DIR}/modules/heat_conduction/test/tests
            specs:
                - tests
            dependencies:
                - framework
            repo: moose
            reports: !include ${MOOSE_DIR}/modules/heat_conduction/doc/sqa_reports.yml

### +`requirement-groups`+

A dictionary that links the default requirement group names to the desired group names. The
[moosesqa/index.md] package automatically groups requirements by top level folders within
the list of directories provided in each category defined in each item of the "categories"
configuration option (see [#categories-config]). When listed this group is used to organize
the requirements. The "requirement-groups" allows the group name to
be modified. For example, [example-req-groups] demonstrates the use of this option.

!listing caption=Example use of "requirement-groups" configuration option. id=example-req-groups
MooseDocs.extensions.sqa:
    requirement-groups:
        dgkernels: DGKernel Objects
        interfacekernels: InterfaceKernel Objects

### +`reports`+

This option provides information for generating of an application level set of SQA reports; please
refer to [python/moosesqa/index.md] for additional details.

### +`default_collection`+

An option that includes a single string that provides the default collection name
(see [#requirements-list]) for requirements. This option defaults to "FUNCTIONAL", thus all requirements
gathered from the specifications are considered to be functional requirements by default.

### +`append_run_exception_to_failure_analysis`+

A boolean that when true (the default) adds the "FAILURE_ANALYSIS" collection (see [#requirements-list])
to all [RunException](framework_stp.md#approval-requirements) tests.

## Requirements List

The `sqa requirements` command will list all the requirements located in the test specifications
for a project; [sqa-requirements] lists all the available settings for this command.

!devel settings id=sqa-requirements
                caption=Configuration options for the SQA extension `requirements` command.
                module=MooseDocs.extensions.sqa
                object=SQARequirementsCommand

The available options for the "collections" settings are listed below.

- +FUNCTIONAL+: Requirements that define uses cases and are correct, unambiguous, complete,
  consistent, verifiable, and traceable.
- +USABILITY+: Requirements for the system that include measurable effectiveness, efficiency, and
  satisfaction criteria in specific contexts of use.
- +PERFORMANCE+: Requirements to define the critical performance conditions and associated
  capabilities.
- +SYSTEM+: Requirements for interfaces among system elements and with external entities.
- +FAILURE_ANALYSIS+: Requirements that perform check for simulation integrity such as error handling
  and convergence failures.

## Extension Commands

The following sections provide information regarding each of the

### Requirement Cross-Reference List

The `sqa cross-reference` command provides a list of requirements in the same fashion as the
[#requirements-list], but organized by design document rather than the folder group.

!devel settings id=sqa-cross-reference
                caption=Configuration options for the SQA extension `cross-reference` command.
                module=MooseDocs.extensions.sqa
                object=SQACrossReferenceCommand

### Verification/Validation List

The `sqa verification` or `sqa validation` command provides a list of requirements in the same
fashion as the [#requirements-list], but organized to only include requirements that include
a "verification" and/or "validation" item in the test specification. [sqa-verification] lists
the available options for these commands. To illustrate the use, the following
specification includes a "verification" item. As such, this requirement would be included in
the requirements list generated by the `sqa verification` command.

```
[Tests]
  [spec]
    type = RunApp
    input = example.i
    verification = 'example.md'
    requirement = "The system shall have an example that demonstrates verification."
  []
[]
```

!devel settings id=sqa-verification
                caption=Configuration options for the SQA extension `verification` and
                        `validation` commands.
                module=MooseDocs.extensions.sqa
                object=SQAVerificationCommand


### Dependencies List

The `sqa dependencies` command creates a list of dependent documents using the information from
the ["categories"](#categories) extension configuration. For example, the [MOOSE SRS](moose_srs.md optional=True) contains
links to the various SRS documents that together form the SQA for the entire MOOSE project. In
general, this command should not be needed by MOOSE-based applications developers that are using the
available templates.

!devel settings id=dependencies
                caption=Configuration options for the SQA extension `dependencies` command.
                module=MooseDocs.extensions.sqa
                object=SQADependenciesCommand


### Document

The `sqa document` command is used to automatically define links to application SQA documents
(e.g., RTM, SRS) for a MOOSE-based application SQA landing page. For example, the document list
on the [Stochastic Tools Module SQA page](stochastic_tools/sqa/index.md optional=True) uses this command.  In
general, this command should not be needed by MOOSE-based applications developers that are using the
available templates. The complete list of available settings for this command are provided in
[document].

!devel settings id=document
                caption=Configuration options for the SQA extension `document` command.
                module=MooseDocs.extensions.sqa
                object=SQADocumentCommand

### Report

The `sqa report` command is used to generate a complete SQA report for a MOOSE-based application
landing page. For example, the report on the
[Stochastic Tools Module SQA page](stochastic_tools/sqa/index.md optional=True) uses this command.  In
general, this command should not be needed by MOOSE-based applications developers that are using the
available templates. The complete list of available settings for this command are provided in
[report].


!devel settings id=report
                caption=Configuration options for the SQA extension `report` command.
                module=MooseDocs.extensions.sqa
                object=SQAReportCommand
