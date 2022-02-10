//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredFlowAreaChange.h"

registerMooseObject("ThermalHydraulicsApp", LayeredFlowAreaChange);

InputParameters
LayeredFlowAreaChange::validParams()
{
  InputParameters params = SideIntegralUserObject::validParams();
  params += LayeredBase::validParams();
  params.addRequiredCoupledVar("displacements",
                               "Displacements, size must match problem dimension.");

  // this layered object should not be used on the displaced mesh
  // because it measures deviation from the undisplaced mesh
  params.suppressParameter<bool>("use_displaced_mesh");
  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<std::vector<SubdomainName>>("block");

  params.addClassDescription(
      "This layered user object computes the change in cross sectional area "
      "of a flow channel from the displacement variables. Note: the convention is"
      "that reduction in flow area is negative. For this to be satisfied, normals must"
      "point INTO the flow channel.");
  return params;
}

LayeredFlowAreaChange::LayeredFlowAreaChange(const InputParameters & parameters)
  : SideIntegralUserObject(parameters), LayeredBase(parameters), _dim(_mesh.dimension())
{
  if (coupledComponents("displacements") != _dim)
    paramError("displacements",
               "The number of displacement components must be equal to the mesh displacement.");

  _disp.resize(_dim);
  for (unsigned int j = 0; j < _dim; ++j)
    _disp[j] = &coupledValue("displacements", j);
}

void
LayeredFlowAreaChange::initialize()
{
  SideIntegralUserObject::initialize();
  LayeredBase::initialize();
}

void
LayeredFlowAreaChange::execute()
{
  unsigned int layer = getLayer(_current_elem->vertex_average());
  Real height = _interval_based ? (_direction_max - _direction_min) / _num_layers
                                : _layer_bounds[layer + 1] - _layer_bounds[layer];
  Real integral_value = computeIntegral() / height;

  setLayerValue(layer, getLayerValue(layer) + integral_value);
}

Real
LayeredFlowAreaChange::computeIntegral()
{
  Real sum = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();

  return sum;
}

Real
LayeredFlowAreaChange::computeQpIntegral()
{
  RealVectorValue displacements;
  for (unsigned int j = 0; j < _dim; ++j)
    displacements(j) = (*_disp[j])[_qp];
  return -_normals[_qp] * displacements;
}

void
LayeredFlowAreaChange::finalize()
{
  LayeredBase::finalize();
}

void
LayeredFlowAreaChange::threadJoin(const UserObject & y)
{
  SideIntegralUserObject::threadJoin(y);
  LayeredBase::threadJoin(y);
}
