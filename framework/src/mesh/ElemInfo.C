//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "FaceInfo.h"
#include "ElemInfo.h"

ElemInfo::ElemInfo(const Elem * const elem)
  : _elem(elem),
    _volume(_elem->volume()),
    _centroid(_elem->vertex_average()),
    _coord_transform_factor(1.0),
    _dof_indices(std::vector<std::vector<dof_id_type>>())
{
}
