!template load file=sqa/rtm.md.template category=python project=MOOSE Tools

!template! item key=minimum_requirements

!! Start with general MOOSE minimum requirements for hardware compatibility

!include sqa/minimum_requirements.md

In addition to this, for {{project}} usage and development, the following is also required:

- A recent version of Python and its interpreter - the last four minor releases are generally supported.
!template-end!

!template! item key=pre-test
!include framework_rtm.md start=pre-test-begin end=pre-test-finish replace=['framework_stp.md', 'python_stp.md']
!template-end!

!template item key=functional-requirements
!sqa requirements collections=FUNCTIONAL category=python

!template item key=usability-requirements
!sqa requirements collections=USABILITY category=python

!template item key=performance-requirements
!sqa requirements collections=PERFORMANCE category=python

!template item key=system-interfaces-requirements
!sqa requirements collections=SYSTEM category=python
