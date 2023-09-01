//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalVoronoiCoupledVoidIC.h"

// MOOSE includes
#include "DelimitedFileReader.h"
#include "GrainTrackerInterface.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "PolycrystalVoronoi.h"

InputParameters
PolycrystalVoronoiCoupledVoidIC::actionParameters()
{
  InputParameters params = InitialCondition::validParams();

  params.addRequiredParam<unsigned int>("op_num", "Number of order parameters");

  params.addParam<bool>(
      "columnar_3D", false, "3D microstructure will be columnar in the z-direction?");

  params.addCoupledVar("v", "Coupled variable");
  params.addRequiredParam<Real>("invalue", "The coupled variable value inside the void");
  params.addRequiredParam<Real>("outvalue", "The coupled variable value inside the void");

  return params;
}

registerMooseObject("PhaseFieldApp", PolycrystalVoronoiCoupledVoidIC);

InputParameters
PolycrystalVoronoiCoupledVoidIC::validParams()
{
  InputParameters params = PolycrystalVoronoiCoupledVoidIC::actionParameters();
  params.addParam<unsigned int>("op_index",
                                0,
                                "The index for the current "
                                "order parameter, not needed if "
                                "structure_type = voids");
  params.addRequiredParam<UserObjectName>(
      "polycrystal_ic_uo", "UserObject for obtaining the polycrystal grain structure.");
  params.addParam<FileName>("file_name",
                            "",
                            "File containing grain centroids, if file_name is "
                            "provided, the centroids "
                            "from the file will be used.");
  return params;
}

PolycrystalVoronoiCoupledVoidIC::PolycrystalVoronoiCoupledVoidIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _op_num(getParam<unsigned int>("op_num")),
    _op_index(getParam<unsigned int>("op_index")),
    _columnar_3D(getParam<bool>("columnar_3D")),
    _poly_ic_uo(getUserObject<PolycrystalVoronoi>("polycrystal_ic_uo")),
    _file_name(getParam<FileName>("file_name")),
    _var_val(coupledValue("v")),
    _invalue(parameters.get<Real>("invalue")),
    _outvalue(parameters.get<Real>("outvalue"))
{
}

void
PolycrystalVoronoiCoupledVoidIC::initialSetup()
{
  if (_op_num <= _op_index)
    mooseError("op_index is too large");

  // Obtain total number and centerpoints of the grains
  _grain_num = _poly_ic_uo.getNumGrains();
  _centerpoints = _poly_ic_uo.getGrainCenters();

  InitialCondition::initialSetup();
}

Real
PolycrystalVoronoiCoupledVoidIC::value(const Point & p)
{
  Real value = 0.0;

  // Determine value for voids
  Real void_value = _var_val[_qp]; // should this be _qp or value(p)
                                   // SpecifiedSmoothCircleIC::value(p);

  // Determine value for grains
  Real grain_value = _poly_ic_uo.getVariableValue(_op_index, p);

  // assigning values for grains (order parameters)
  if (grain_value == 0) // Not in this grain
    value = grain_value;
  else                           // in this grain, but might be in a void
    if (void_value == _outvalue) // Not in a void
      value = grain_value;
    else if (void_value > _outvalue && void_value < _invalue) // On void interface
      value = grain_value * (_invalue - void_value) / (_invalue - _outvalue);
    else if (void_value == _invalue) // In a void, so op = 0
      value = 0.0;

  return value;
}
