# EBR-II SHRT-17 steady state of XX09_SCM_SS17.i with the Pacio-Chen-Todreas friction and
# mixing parameterizations

!include XX09_SCM_SS17.i

[SCMClosures]
  [Chen]
    friction_model := Pacio
  []
  [Chen_Todreas]
    mixing_model := Pacio
  []
[]
