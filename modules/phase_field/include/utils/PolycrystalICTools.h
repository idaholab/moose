#ifndef POLYCRYSTALICTOOLS_H
#define POLYCRYSTALICTOOLS_H

#include "Moose.h"
#include "libmesh/libmesh.h"
#include "InitialCondition.h"

namespace PolycrystalICTools
{
std::vector<Real>
assignPointsToVariables(const std::vector<Point> centerpoints,
                        const Real op_num,
                        const MooseMesh & mesh,
                        const MooseVariable & var);

unsigned int
assignPointToGrain(const Point & p,
                   const std::vector<Point> centerpoints,
                   const MooseMesh & mesh,
                   const MooseVariable & var,
                   const Real maxsize);
}


#endif //POLYCRYSTALICTOOLS_H
