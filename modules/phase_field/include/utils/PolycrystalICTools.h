/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALICTOOLS_H
#define POLYCRYSTALICTOOLS_H

#include "Moose.h"
#include "libmesh/libmesh.h"
#include "InitialCondition.h"

namespace PolycrystalICTools
{
std::vector<unsigned int>
assignPointsToVariables(const std::vector<Point> & centerpoints,
                        const Real op_num,
                        const MooseMesh & mesh,
                        const MooseVariable & var);

unsigned int
assignPointToGrain(const Point & p,
                   const std::vector<Point> & centerpoints,
                   const MooseMesh & mesh,
                   const MooseVariable & var,
                   const Real maxsize);
}


#endif //POLYCRYSTALICTOOLS_H
