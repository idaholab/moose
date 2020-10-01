//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadialDisplacementCylinderAux.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", RadialDisplacementCylinderAux);

InputParameters
RadialDisplacementCylinderAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Compute the radial component of the displacement vector for cylindrical models.");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<RealVectorValue>(
      "origin", "Origin of cylinder axis of rotation for 2D and 3D Cartesian models");
  params.addParam<RealVectorValue>(
      "axis_vector", "Vector defining direction of cylindrical axis (3D Cartesian models)");
  params.set<bool>("use_displaced_mesh") = false;

  return params;
}

RadialDisplacementCylinderAux::RadialDisplacementCylinderAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp_vals(coupledValues("displacements"))
{
  const std::set<SubdomainID> & subdomains = _mesh.meshSubdomains();
  const auto & sbd_begin = *subdomains.begin();
  for (const auto & sbd : subdomains)
  {
    if (sbd == sbd_begin)
      _coord_system = _subproblem.getCoordSystem(sbd);
    else if (_subproblem.getCoordSystem(sbd) != _coord_system)
      mooseError("RadialDisplacementCylinderAux requires that all subdomains have the same "
                 "coordinate type");
  }

  if (_ndisp != _mesh.dimension())
    mooseError("The number of displacement variables supplied must match the mesh dimension.");

  if (_coord_system == Moose::COORD_XYZ && _ndisp == 1)
    mooseError("RadialDisplacmentCylinderAux is not applicable for 1D Cartesian models");

  else if (!(_coord_system == Moose::COORD_XYZ || _coord_system == Moose::COORD_RZ))
    mooseError("RadialDisplacementCylinderAux can only be used with Cartesian or axisymmetric "
               "coordinate systems");

  if (isParamValid("origin"))
  {
    if (_coord_system != Moose::COORD_XYZ)
      mooseError("The 'origin' parameter is only valid for Cartesian models.");

    _origin = getParam<RealVectorValue>("origin");
  }
  else if (_coord_system == Moose::COORD_XYZ)
    mooseError("Must specify 'origin' for models with Cartesian coordinate systems.");

  if (isParamValid("axis_vector"))
  {
    if (!(_coord_system == Moose::COORD_XYZ && _ndisp == 3))
      mooseError("The 'axis_vector' parameter is only valid for 3D Cartesian models.");

    _axis_vector = getParam<RealVectorValue>("axis_vector");
    Real vec_len = _axis_vector.norm();
    if (MooseUtils::absoluteFuzzyEqual(vec_len, 0.0))
      mooseError("axis_vector must have nonzero length");
    _axis_vector /= vec_len;
  }
  else if (_coord_system == Moose::COORD_XYZ && _ndisp == 3)
    mooseError("Must specify 'axis_vector' for 3D Cartesian models");

  if (!isNodal())
    mooseError("Must run on a nodal variable");
}

Real
RadialDisplacementCylinderAux::computeValue()
{
  Real rad_disp = 0.0;
  Point current_point(*_current_node);

  switch (_coord_system)
  {
    case Moose::COORD_XYZ:
    {
      RealVectorValue rad_vec;
      const RealVectorValue disp_vec((*_disp_vals[0])[_qp],
                                     (*_disp_vals[1])[_qp],
                                     (_ndisp == 3 ? (*_disp_vals[2])[_qp] : 0.0));

      if (_ndisp == 2)
        rad_vec = current_point - _origin;
      else if (_ndisp == 3)
      {
        // t is the distance along the axis from point 1 to 2 to the point nearest to the current
        // point.
        const RealVectorValue p1pc(current_point - _origin);
        const Real t = p1pc * _axis_vector;

        // The nearest point on the cylindrical axis to current_point is p.
        const RealVectorValue p(_origin + t * _axis_vector);
        rad_vec = current_point - p;
      }

      Real rad = rad_vec.norm();
      if (rad > 0.0)
      {
        rad_vec /= rad;
        rad_disp = rad_vec * disp_vec;
      }
      else
        rad_disp = disp_vec.norm();
      break;
    }
    case Moose::COORD_RZ:
      rad_disp = (*_disp_vals[0])[_qp];
      break;
    default:
      mooseError("Unsupported coordinate system");
  }

  return rad_disp;
}
