//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointInPolyhedronCheckUO.h"

// Register object
registerMooseObject("ShiftedBoundaryMethodApp", PointInPolyhedronCheckUO);

InputParameters
PointInPolyhedronCheckUO::validParams()
{
  InputParameters params = PointInPolyhedronBaseUO::validParams();
  params.addClassDescription("InOut test based on boundary elements from SBMSurfaceMeshBuilder.");

  params.addRequiredParam<UserObjectName>("builder",
                                          "The SBMSurfaceMeshBuilder providing boundary elements.");

  return params;
}

PointInPolyhedronCheckUO::PointInPolyhedronCheckUO(const InputParameters & parameters)
  : PointInPolyhedronBaseUO(parameters),
    _builder(getUserObject<SBMSurfaceMeshBuilder>("builder")),
    _in_out_test_struct(nullptr)
{
}

void
PointInPolyhedronCheckUO::initialSetup()
{
  _in_out_test_struct = std::make_unique<PointInPolyhedronCheck>(_builder.getBoundaryElements(),
                                                                 _builder.getCentroids(),
                                                                 _ray_direction,
                                                                 _brute_force,
                                                                 _eps,
                                                                 _leaf_max_size,
                                                                 _obb_file_name,
                                                                 _ray_file_name);
}
