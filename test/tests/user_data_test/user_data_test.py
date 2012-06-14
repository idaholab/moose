from options import *

test = {
    INPUT : 'user_data_test.i',
    EXODIFF : ['out.e']
}

restart_ud_test_1 = {
  INPUT : 'ud_restart_part1.i',
  EXODIFF : ['ud_restart_part1_out.e']
}

restart_ud_test_2 = {
  INPUT : 'ud_restart_part2.i',
  EXODIFF : ['ud_restart_part2_out.e'],
  PREREQ : 'restart_ud_test_1'
}
