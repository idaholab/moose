#!/usr/bin/python


''' Merge the lists together preserving the ordering of master.  "master" will be the merged list after completion. '''
def mergeLists(master, slave):
  for slave_value_position in xrange(0,len(slave)):
    value = slave[slave_value_position]
    if value in master:
      continue
    else:
      slave_before_slice = slave[0:slave_value_position]
      slave_after_slice = slave[slave_value_position+1:]
      print value
      print slave_before_slice
      print slave_after_slice

      best_score = 0
      best_position = 0

      for master_position in xrange(0,len(master)):
        current_score = 0
        master_before_slice = master[0:master_position]
        master_after_slice = master[master_position:]
        print 'master_position',master_position
        print 'master_before',master_before_slice
        print 'master_after',master_after_slice

        for slave_slice_position in xrange(len(slave_before_slice)):
          slave_value = slave_before_slice[slave_slice_position]
          if slave_value in master_before_slice:
            current_score += 1.0/float(len(slave_before_slice)-slave_slice_position+1)
            print master_position, current_score

        for slave_slice_position in xrange(len(slave_after_slice)):
          slave_value = slave_after_slice[slave_slice_position]
          if slave_value in master_after_slice:
            current_score += 1.0/float(slave_slice_position+1)
            print master_position, current_score

        if current_score > best_score:
          best_position = master_position
          best_score = current_score
      master.insert(best_position,value)


if __name__ == '__main__':
#  input = ['Variables','Functions','Kernels','BCs','Executioner','Output']
  input = []
#  template = ['Variables','AuxVariables','Kernels','AuxKernels','BCs','AuxBCs','Postprocessors','Executioner','Output']
  template = ['Variables','AuxVariables']
  mergeLists(input, template)
  print input
