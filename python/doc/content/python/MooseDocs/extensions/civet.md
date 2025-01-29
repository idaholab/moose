# CIVET Extension

The [!ac](CIVET) extension adds the ability for MooseDocs to download results from a [CIVET] client
(e.g., [https://civet.inl.gov](https://civet.inl.gov)) and present them as reports, links, badges,
and other information in a MooseDocs page. This extension is most-often used with the [extensions/sqa.md].

## Extension Configuration

[config-civet-ext] lists the available configuration settings for the CIVET extension. These
configuration items should be included in the MooseDocs configuration file (e.g., config.yml).

!devel settings id=config-civet-ext
                caption=Configuration options for the CIVET extension.
                module=MooseDocs.extensions.civet
                object=CivetExtension

## Extension Commands

The following sections provide information regarding each of the commands available to the extension.

### Results Inline Linking

The `results` inline command allows for linking to testing results for the version of the code repository
used to build the documentation. It generates a single link displaying the git commit SHA which links
to the associated CIVET results for that SHA.

!devel! example id=civet-results-example
                caption=Example of the CIVET extension `results` command.

[!civet!results]

!devel-end!

!devel settings id=civet-results
                caption=Options for the CIVET extension `results` command.
                module=MooseDocs.extensions.civet
                object=CivetResultsCommand

### Merge Results Linking

The `civet mergeresults` command provides a set of links to CIVET results pages for the version of
the code repository used to build the documentation (by default). This is similar to the [#results-inline-linking]
command, except multiple links are provided. Each link is labeled using the git SHA associated with
the merge event that prompted testing. This means that MOOSE, for example, might have multiple links
associated with `next`, `devel`, and `master` testing events associated with a single `master` branch
merge constituting the current version of the code and documentation. If the `use_current_hash`
settings option is set to `False`, then the most up-to-date version of the these links is retrieved
from the online remote repository.

!devel! example id=civet-merge-results-example
                caption=Example of the CIVET extension `mergeresults` command.

!civet mergeresults
!devel-end!

!alert! tip title=Inline usage
Note that this command can also be used inline, using the `[!civet!mergeresults]` syntax.
!alert-end!

!devel settings id=civet-merge-results
                caption=Options for the CIVET extension `mergeresults` command.
                module=MooseDocs.extensions.civet
                object=CivetMergeResultsCommand

### Test Results Badges

The `badges` command allows the display of a badge (or badges) containing the aggregate status of
all tests that were completed corresponding to one or more test specifications for the results
associated with the version of the code repository used to build the documentation. Clicking on the
badge directs the browser to a report page where individual results can be inspected.

!devel! example id=civet-badges-example
                caption=Examples of the CIVET extension `badges` command.

With one test specification:

!civet badges tests=kernels/simple_diffusion.test

With two test specifications:

!civet badges tests=kernels/simple_diffusion.test outputs/common.exodus

!devel-end!

!alert! tip title=Inline usage
Note that this command can also be used inline, using the `[!civet!badges tests=...]` syntax.
!alert-end!

!devel settings id=civet-badges
                caption=Options for the CIVET extension `badges` command.
                module=MooseDocs.extensions.civet
                object=CivetTestBadgesCommand

### Test Results Report

The `report` command generates a table of jobs, associated CIVET recipes, and test statuses for a given test specification. While this command is available for general use within any markdown page, the table generation activity called by this command is generally used within the [#test-results-badges] command, where a results page is generated for linking to the rendered badge.

!devel! example id=civet-report-example
                caption=Example of the CIVET extension `report` command.

!civet report tests=kernels/simple_diffusion.test

!devel-end!

!devel settings id=civet-report
                caption=Options for the CIVET extension `report` command.
                module=MooseDocs.extensions.civet
                object=CivetTestReportCommand
