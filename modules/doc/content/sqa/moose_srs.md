!template load file=sqa/app_srs.md.template app=MOOSE category=_empty_

!template item key=system-purpose
!include system_purpose.md

!template item key=system-scope
!include system_scope.md

!template item key=system-context
!include framework_srs.md start=system-context-begin end=system-context-finish

!template item key=user-characteristics
!include framework_srs.md start=user-characteristics-begin end=user-characteristics-finish

!template! item key=assumptions-and-dependencies
!include sqa/assumptions_and_dependencies.md
!template-end!

!template item key=packaging
!include framework_srs.md start=packaging-begin end=packaging-finish

!template item key=functional-requirements
All of the requirements for [!ac](MOOSE) are provided in the dependent [!ac](SRS) documents, please
refer to the documents listed in [#assumptions-and-dependencies].

!template item key=usability-requirements
All of the requirements for [!ac](MOOSE) are provided in the dependent [!ac](SRS) documents, please
refer to the documents listed in [#assumptions-and-dependencies].

!template item key=performance-requirements
All of the requirements for [!ac](MOOSE) are provided in the dependent [!ac](SRS) documents, please
refer to the documents listed in [#assumptions-and-dependencies].

!template item key=system-interfaces-requirements
All of the requirements for [!ac](MOOSE) are provided in the dependent [!ac](SRS) documents, please
refer to the documents listed in [#assumptions-and-dependencies].

!template item key=reliability
!include framework_srs.md start=reliability-begin end=reliability-finish

!template item key=information-management
!include framework_srs.md start=information-management-begin end=information-management-finish

!template item key=policies-and-regulations
!include framework_srs.md start=policies-and-regulations-begin end=policies-and-regulations-finish
