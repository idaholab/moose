!template load file=sqa/module_rtm.md.template module=Thermal Hydraulics category=thermal_hydraulics

!template! item key=system-purpose
!include thermal_hydraulics_srs.md start=system-purpose-begin end=system-purpose-finish
!template-end!

!template! item key=system-scope
!include thermal_hydraulics_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=log-revisions
The following known log revisions exist for the {{module}} module:

!table
| MOOSE Commit | Incorrect Referenced Issue | Correct Referenced Issue |
| - | - | - |
| [48db61](https://github.com/idaholab/moose/commit/48db61307ed87b58a96e944215f13378138cf7bc) | idaholab/sockeye#19845 | [idaholab/moose#19845](https://github.com/idaholab/moose/pull/19845) |

!template-end!
