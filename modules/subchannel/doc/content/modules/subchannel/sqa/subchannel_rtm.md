!template load file=sqa/module_rtm.md.template category=subchannel module=SubChannel

!template! item key=system-purpose
!include subchannel_srs.md start=system-purpose-begin end=system-purpose-finish
!template-end!

!template! item key=system-scope
!include subchannel_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=log-revisions
The changelog for all code residing in the MOOSE repository is located in the
[MOOSE RTM](moose_rtm.md#log-revisions optional=True).
!template-end!
