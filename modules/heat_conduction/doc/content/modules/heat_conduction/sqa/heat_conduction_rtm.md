!template load file=sqa/module_rtm.md.template category=heat_conduction module=Heat Conduction stp_filename=heat_conduction_stp.md

!template! item key=system-purpose
!include heat_conduction_srs.md start=system-purpose-begin end=system-purpose-finish
!template-end!

!template! item key=system-scope
!include heat_conduction_srs.md start=system-scope-begin end=system-scope-finish
!template-end!
