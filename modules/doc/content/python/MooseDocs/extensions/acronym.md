# Acronym Extension

The acronym extension provides a convenient way to defined acronyms that may be used throughout all
documentation pages. On each page the full text and acronym definition will be displayed the first
time is used, as shown in [acronym-example].

!alert note
When viewing on the website a tooltip is created for the the acronyms, the
full name will appear when the mouse is placed over the acronym.

!devel example id=acronym-example caption=Example use of acronyms.
The [!ac](INL) was founded in 1949 and has changed names many times, but [!ac](INL) has
been used since 2005.

## Acronym List

A list of acronyms can be created by using the `!acronym list` command. [acronym-list-settings]
show the available options for this command.

!devel settings
       module=MooseDocs.extensions.acronym
       object=AcronymListComponent
       id=acronym-list-settings
       caption=Available configuration options for the `!acronym list` command.


!devel! example id=acronym-list-example caption=Example of the `!acronym list` command.
!acronym list complete=True
!devel-end!

## Configuration

The available configuration options for the acronym extension are provided in [acronym-ext-config].

!devel settings
       module=MooseDocs.extensions.acronym
       object=AcronymExtension
       id=acronym-ext-config
       caption=Available configuration options for the AcronymExtension object.

The acronyms are defined in the configuration (e.g., `config.yml`) as a dictionary of items
under the "acronyms" configuration item.

```yaml
Extensions:
    acronym:
        type: MooseDocs.extensions.acronym
        acronyms:
            INL: Idaho National Laboratory
            INEL: Idaho National Engineering Laboratory
            INEEL: Idaho National Engineering and Environmental Laboratory
```

Additionally, multiple dictionaries may be used to allow items to be pulled from
multiple sources. For example, to use the acronyms defined within the framework as well as
define additional items, the following may be done.

```yaml
Extensions:
    acronym:
        type: MooseDocs.extensions.acronym
        acronyms:
            framework: !include ${MOOSE_DIR}/framework/doc/acronyms.yml
            my_app:
                INEL: Idaho National Engineering Laboratory
                INEEL: Idaho National Engineering and Environmental Laboratory
```
