!template load file=sqa/module_rtm.md.template category=reactor module=Reactor stp_filename=reactor_stp.md

!template! item key=system-purpose
!include reactor_srs.md start=system-purpose-begin end=system-purpose-finish
!template-end!

!template! item key=system-scope
!include reactor_srs.md start=system-scope-begin end=system-scope-finish
!template-end!
