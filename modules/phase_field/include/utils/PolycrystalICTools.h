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
std::vector<unsigned>
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

std::vector<std::vector<bool> >
buildGrainAdjacencyGraph(const std::map<dof_id_type, unsigned int> & node_to_grain,
                         MooseMesh & mesh,
                         unsigned int n_grains);

std::vector<unsigned int>
assignOpsToGrains(const std::vector<std::vector<bool> > & adjacency_matrix,
                  unsigned int n_grains,
                  unsigned int n_ops);
}


#endif //POLYCRYSTALICTOOLS_H
