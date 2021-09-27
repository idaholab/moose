# Coverage Extension

The coverage extension provides a means for presenting data from from a config file. The available
settings for the extension are provided in [coverage-extension].

!devel settings module=MooseDocs.extensions.coverage
                object=CoverageExtension
                id=coverage-extension
                caption=Configuration items for the coverage extension.

## Config Table

A complete table of the contents of a config file can be presented as a table as shown in
[example-coverage-table] and the available options in [coverage-table-settings].

!devel! example id=example-coverage-table caption=Example creation of table config file contents.
!coverage table
!devel-end!

!devel settings module=MooseDocs.extensions.coverage
                object=CoverageTableCommand
                id=coverage-table-settings
                caption=Available settings for the coverage table command.

## Config Value

A single falue from the a config file can be extracted as as a table as shown in
[example-coverage-value] and the available options in [coverage-value-settings].

!devel! example id=example-coverage-value caption=Example creation of table config file contents.
The combined module requires [!coverage!value section=combined option=require_total]% line coverage.
!devel-end!

!devel settings module=MooseDocs.extensions.coverage
                object=CoverageValueCommand
                id=coverage-value-settings
                caption=Available settings for the coverage value command.
