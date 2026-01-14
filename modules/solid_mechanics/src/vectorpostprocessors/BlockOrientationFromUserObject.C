//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockOrientationFromUserObject.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "SystemBase.h"
#include "libmesh/quadrature.h"
#include "EulerAngles.h"

registerMooseObject("SolidMechanicsApp", BlockOrientationFromUserObject);

InputParameters
BlockOrientationFromUserObject::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addRequiredParam<UserObjectName>(
      "block_orientation_uo",
      "Name of ComputeBlockOrientation user object for updated block orientation.");

  params.addParam<bool>(
      "degree_to_radian",
      false,
      "Whether to convert euler angles from degree to radian. The default is to use degrees.");

  params.addClassDescription(
      "Output the Euler angle for each block computed from average of quaternions.");
  return params;
}

BlockOrientationFromUserObject::BlockOrientationFromUserObject(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _uo_name(getParam<UserObjectName>("block_orientation_uo")),
    _num_cols(4), // add one colum for the subdomain ID
    _num_rows(_mesh.meshSubdomains().size())
{
  _output_vector.resize(_num_cols);

  for (const auto j : make_range(_num_cols))
  {
    if (j == 0)
      _output_vector[j] = &declareVector("subdomain_id");
    else
      _output_vector[j] = &declareVector("euler_angle_" + std::to_string(j)); // can change
  }

  _uo = &getUserObjectByName<ComputeBlockOrientationBase>(_uo_name);
}

void
BlockOrientationFromUserObject::initialize()
{
  for (const auto j : make_range(_num_cols))
  {
    _output_vector[j]->clear();
    _output_vector[j]->resize(_num_rows, 0.0);
  }
}

void
BlockOrientationFromUserObject::finalize()
{
  // parallel communication
  for (const auto row : make_range(_num_rows))
    for (const auto col : make_range(_num_cols))
      _communicator.max((*_output_vector[col])[row]);
}

void
BlockOrientationFromUserObject::execute()
{
  int row = 0;
  for (const auto sid : _mesh.meshSubdomains())
  {
    // get Euler angle for each subdomain
    EulerAngles ea = _uo->getBlockOrientation(sid);

    // convert EulerAngles to RealVectorValue
    RealVectorValue euler_angle = (RealVectorValue)ea;

    if (getParam<bool>("degree_to_radian"))
      euler_angle *= pi / 180.0;

    for (const auto col : make_range(_num_cols))
    {
      if (col == 0)
        (*_output_vector[col])[row] = sid;
      else
        (*_output_vector[col])[row] = euler_angle(col - 1);
    }
    row++;
  }
}
