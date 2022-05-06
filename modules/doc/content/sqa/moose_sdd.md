!template load file=sqa/app_sdd.md.template app=MOOSE category=_empty_

!template item key=introduction
!include framework_sdd.md start=introduction-begin end=introduction-finish

!template item key=system-scope
!include framework_sdd.md start=system-scope-begin end=system-scope-finish

!template item key=dependencies-and-limitations
!include framework_sdd.md start=dependencies-and-limitations-begin end=dependencies-and-limitations-finish

!template item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish

!template item key=system-design
!include framework_sdd.md start=system-design-begin end=system-design-finish

!template item key=system-structure
!include framework_sdd.md start=system-structure-begin end=system-structure-finish

!template! item key=requirements-cross-reference
All of the design documents for [!ac](MOOSE) are provided in the dependent [!ac](SDD) documents,
please refer to the documents listed below.

!sqa dependencies suffix=sdd category={{category}}
!template-end!
