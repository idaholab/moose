#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import math
import numpy

def dataUnion(data_list, tolerance=1e-9):

  # create sorted copies of the data and a union of the x points
  sorted_data = []
  union_x = []
  for data in data_list:
    union_x = union_x + data[0]
    sorted_data.append((sorted(data[0]), [x for _,x in sorted(zip(data[0],data[1]))]))

  # quit early if no work is to be done
  if not union_x:
      return ([], [[]] * len(data_list))

  # prune union using tolerance
  union_x = sorted(union_x)
  pruned_x = [union_x[0]]
  if len(union_x) > 1:
    for i in range(1, len(union_x)):
      if math.fabs(union_x[i] - union_x[i-1]) >= tolerance:
        pruned_x.append(union_x[i])

  # now interpolate all sets at the pruned x points
  interpolated_ys = []
  for data in sorted_data:
    interpolated_ys.append(numpy.interp(pruned_x, data[0], data[1], numpy.nan, numpy.nan).tolist())

  return (pruned_x, interpolated_ys)
