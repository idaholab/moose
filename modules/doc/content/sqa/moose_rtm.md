!template load file=sqa/app_rtm.md.template app=MOOSE category=moose

!template item key=system-purpose
!include system_purpose.md

!template item key=system-scope
!include system_scope.md

!template! item key=log-revisions
The following known log revisions exist for the {{module}} module:

!table
| MOOSE Commit | Incorrect Referenced Issue | Correct Referenced Issue |
| - | - | - |
| [48db61](https://github.com/idaholab/moose/commit/48db61307ed87b58a96e944215f13378138cf7bc) | idaholab/sockeye#19845 | [idaholab/sockeye#130](https://github.inl.gov/ncrc/sockeye/issues/130) |

!template-end!

!template item key=functional-requirements
All of the requirements for [!ac](MOOSE) are provided in the dependent [!ac](RTM) documents, please
refer to the documents listed in [#assumptions-and-dependencies].

!template item key=usability-requirements
All of the requirements for [!ac](MOOSE) are provided in the dependent [!ac](RTM) documents, please
refer to the documents listed in [#assumptions-and-dependencies].

!template item key=performance-requirements
All of the requirements for [!ac](MOOSE) are provided in the dependent [!ac](RTM) documents, please
refer to the documents listed in [#assumptions-and-dependencies].

!template item key=system-interfaces-requirements
All of the requirements for [!ac](MOOSE) are provided in the dependent [!ac](RTM) documents, please
refer to the documents listed in [#assumptions-and-dependencies].
