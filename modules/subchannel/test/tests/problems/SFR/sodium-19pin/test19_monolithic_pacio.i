# test19_monolithic.i with the Pacio-Chen-Todreas friction and mixing parameterizations

!include test19_monolithic.i

[SCMClosures]
  [Chen]
    friction_model := Pacio
  []
  [Chen_Todreas]
    mixing_model := Pacio
  []
[]
