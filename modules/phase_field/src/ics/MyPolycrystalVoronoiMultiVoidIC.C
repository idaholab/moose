//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MyPolycrystalVoronoiMultiVoidIC.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "DelimitedFileReader.h"
#include "GrainTrackerInterface.h"
#include "PolycrystalVoronoi.h"

InputParameters
MyPolycrystalVoronoiMultiVoidIC::actionParameters()
{
  InputParameters params = ::validParams<MultiSmoothCircleIC>();

  params.addRequiredParam<unsigned int>("op_num", "Number of order parameters");

  params.addParam<bool>(
      "columnar_3D", false, "3D microstructure will be columnar in the z-direction?");

  return params;
}

registerMooseObject("PhaseFieldApp", MyPolycrystalVoronoiMultiVoidIC);

InputParameters
MyPolycrystalVoronoiMultiVoidIC::validParams()
{
  InputParameters params = MyPolycrystalVoronoiMultiVoidIC::actionParameters();
  MooseEnum structure_options("grains voids");
  params.addRequiredParam<MooseEnum>("structure_type",
                                     structure_options,
                                     "Which structure type is being initialized, grains or voids");
  params.addParam<unsigned int>("op_index",
                                0,
                                "The index for the current "
                                "order parameter, not needed if "
                                "structure_type = voids");
  params.addRequiredParam<UserObjectName>(
      "polycrystal_ic_uo", "UserObject for obtaining the polycrystal grain structure.");
  params.addParam<FileName>(
      "file_name",
      "",
      "File containing grain centroids, if file_name is provided, the centroids "
      "from the file will be used.");
  return params;
}

MyPolycrystalVoronoiMultiVoidIC::MyPolycrystalVoronoiMultiVoidIC(const InputParameters & parameters)
  : MultiSmoothCircleIC(parameters),
    _structure_type(getParam<MooseEnum>("structure_type")),
    _op_num(getParam<unsigned int>("op_num")),
    _op_index(getParam<unsigned int>("op_index")),
    _columnar_3D(getParam<bool>("columnar_3D")),
    _poly_ic_uo(getUserObject<PolycrystalVoronoi>("polycrystal_ic_uo")),
    _file_name(getParam<FileName>("file_name"))
{
  if (_invalue < _outvalue)
    mooseError("MyPolycrystalVoronoiMultiVoidIC requires that the voids be "
               "represented with invalue > outvalue");
  if (_numbub == 0)
    mooseError("MyPolycrystalVoronoiMultiVoidIC requires numbub > 0. If you want no voids to "
               "be "
               "represented, use invalue = outvalue. In general, you should use "
               "PolycrystalVoronoi to represent Voronoi grain structures without "
               "voids.");
}

void
MyPolycrystalVoronoiMultiVoidIC::initialSetup()
{
  if (_op_num <= _op_index)
    mooseError("op_index is too large in CircleGrainVoidIC");

  // Obtain total number and centerpoints of the grains
  _grain_num = _poly_ic_uo.getNumGrains();
  _centerpoints = _poly_ic_uo.getGrainCenters();

  // Call initial setup from MultiSmoothCircleIC to create _centers and _radii
  // for voids
  MultiSmoothCircleIC::initialSetup();
}

Real
MyPolycrystalVoronoiMultiVoidIC::value(const Point & p)
{
  Real value = 0.0;

  // Determine value for voids
  Real void_value = MultiSmoothCircleIC::value(p);

  // Determine value for grains
  Real grain_value = _poly_ic_uo.getVariableValue(_op_index, p);

  switch (_structure_type)
  {
    case 0:                 // assigning values for grains (order parameters)
      if (grain_value == 0) // Not in this grain
        value = grain_value;
      else                             // in this grain, but might be in a void
          if (void_value == _outvalue) // Not in a void
        value = grain_value;
      else if (void_value > _outvalue && void_value < _invalue) // On void interface
        value = grain_value * (_invalue - void_value) / (_invalue - _outvalue);
      else if (void_value == _invalue) // In a void, so op = 0
        value = 0.0;
      break;

    case 1: // assigning values for voids (concentration)
      value = void_value;
      break;
  }

  return value;
}

RealGradient
MyPolycrystalVoronoiMultiVoidIC::gradient(const Point & p)
{
  RealGradient gradient;
  RealGradient void_gradient = MultiSmoothCircleIC::gradient(p);

  // Order parameter assignment assumes zero gradient (sharp interface)
  switch (_structure_type)
  {
    case 1: // assigning gradient for voids
      gradient = void_gradient;
      break;
  }

  return gradient;
}
