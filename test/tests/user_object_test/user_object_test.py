from options import *

test = {
    INPUT : 'user_object_test.i',
    EXODIFF : ['out.e']
}

restart_uo_test_1 = {
  INPUT : 'uo_restart_part1.i',
  EXODIFF : ['uo_restart_part1_out.e']
}

restart_uo_test_2 = {
  INPUT : 'uo_restart_part2.i',
  EXODIFF : ['uo_restart_part2_out.e'],
  PREREQ : ['restart_uo_test_1']
}
