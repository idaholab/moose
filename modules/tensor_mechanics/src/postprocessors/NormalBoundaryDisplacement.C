//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalBoundaryDisplacement.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", NormalBoundaryDisplacement);

InputParameters
NormalBoundaryDisplacement::validParams()
{
  InputParameters params = SidePostprocessor::validParams();
  params.addRequiredCoupledVar("displacements", "The names of the displacement variables");
  MooseEnum type_options("average=0 absolute_average=1 max=2 absolute_max=3");
  params.addRequiredParam<MooseEnum>(
      "value_type", type_options, "Type of extreme value to return.");
  params.addClassDescription(
      "This postprocessor computes the normal displacement on a given set of boundaries.");
  return params;
}

NormalBoundaryDisplacement::NormalBoundaryDisplacement(const InputParameters & parameters)
  : SidePostprocessor(parameters),
    _value_type(getParam<MooseEnum>("value_type").getEnum<NormalValType>()),
    _ncomp(coupledComponents("displacements"))
{
  if (_ncomp != _mesh.dimension())
    paramError("displacements", "Number of entries must match the mesh dimension.");

  _disp.resize(_ncomp);
  for (unsigned int j = 0; j < _ncomp; ++j)
    _disp[j] = &coupledValue("displacements", j);
}

void
NormalBoundaryDisplacement::initialize()
{
  _boundary_displacement = 0;
  _area = 0;
}

void
NormalBoundaryDisplacement::execute()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    Real ddn = 0;
    for (unsigned int j = 0; j < _ncomp; ++j)
      ddn += _normals[qp](j) * (*_disp[j])[qp];

    _area += _JxW[qp] * _coord[qp];
    switch (_value_type)
    {
      case NormalValType::AVERAGE:
        _boundary_displacement += _JxW[qp] * _coord[qp] * ddn;
        break;
      case NormalValType::ABSOLUTE_AVERAGE:
        _boundary_displacement += _JxW[qp] * _coord[qp] * std::abs(ddn);
        break;
      case NormalValType::MAX:
        if (ddn > _boundary_displacement)
          _boundary_displacement = ddn;
        break;
      case NormalValType::ABSOLUTE_MAX:
        ddn = std::abs(ddn);
        if (ddn > _boundary_displacement)
          _boundary_displacement = ddn;
        break;
    }
  }
}

Real
NormalBoundaryDisplacement::getValue()
{
  return _boundary_displacement;
}

void
NormalBoundaryDisplacement::threadJoin(const UserObject & y)
{
  const NormalBoundaryDisplacement & pps = static_cast<const NormalBoundaryDisplacement &>(y);

  switch (_value_type)
  {
    case NormalValType::AVERAGE:
      _boundary_displacement += pps._boundary_displacement;
      break;
    case NormalValType::ABSOLUTE_AVERAGE:
      _boundary_displacement += pps._boundary_displacement;
      break;
    case NormalValType::MAX:
      if (pps._boundary_displacement > _boundary_displacement)
        _boundary_displacement = pps._boundary_displacement;
      break;
    case NormalValType::ABSOLUTE_MAX:
      if (pps._boundary_displacement > _boundary_displacement)
        _boundary_displacement = pps._boundary_displacement;
      break;
  }

  _area += pps._area;
}

void
NormalBoundaryDisplacement::finalize()
{
  gatherSum(_area);
  switch (_value_type)
  {
    case NormalValType::AVERAGE:
      gatherSum(_boundary_displacement);
      _boundary_displacement /= _area;
      break;
    case NormalValType::ABSOLUTE_AVERAGE:
      gatherSum(_boundary_displacement);
      _boundary_displacement /= _area;
      break;
    case NormalValType::MAX:
      gatherMax(_boundary_displacement);
      break;
    case NormalValType::ABSOLUTE_MAX:
      gatherMax(_boundary_displacement);
      break;
  }
}
