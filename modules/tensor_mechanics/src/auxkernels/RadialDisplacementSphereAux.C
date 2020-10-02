//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadialDisplacementSphereAux.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", RadialDisplacementSphereAux);

InputParameters
RadialDisplacementSphereAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Compute the radial component of the displacement vector for spherical models.");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<RealVectorValue>("origin",
                                   "Sphere origin for 3D Cartesian and 2D axisymmetric models");
  params.set<bool>("use_displaced_mesh") = false;

  return params;
}

RadialDisplacementSphereAux::RadialDisplacementSphereAux(const InputParameters & parameters)
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
      mooseError(
          "RadialDisplacementSphereAux requires that all subdomains have the same coordinate type");
  }

  if (_ndisp != _mesh.dimension())
    mooseError("The number of displacement variables supplied must match the mesh dimension.");

  if ((_coord_system == Moose::COORD_XYZ) || (_coord_system == Moose::COORD_RZ))
  {
    if (isParamValid("origin"))
      _origin = getParam<RealVectorValue>("origin");
    else
      mooseError(
          "Must specify 'origin' for models with Cartesian or axisymmetric coordinate systems.");
  }
  else if (isParamValid("origin"))
    mooseError("The 'origin' parameter is only valid for models with Cartesian or axisymmetric "
               "coordinate systems.");

  if (_coord_system == Moose::COORD_XYZ && _ndisp != 3)
    mooseError("Cannot compute radial displacement for models with 1D or 2D Cartesian system");

  if (_coord_system == Moose::COORD_RZ && _ndisp != 2)
    mooseError(
        "Can only compute radial displacement for axisymmetric systems if the dimensionality is 2");

  if (!isNodal())
    mooseError("Must run on a nodal variable");
}

Real
RadialDisplacementSphereAux::computeValue()
{
  Real rad_disp = 0.0;

  if ((_coord_system == Moose::COORD_XYZ && _ndisp == 3) ||
      (_coord_system == Moose::COORD_RZ && _ndisp == 2))
  {
    Point current_point(*_current_node);
    RealVectorValue rad_vec(current_point - _origin);
    Real rad = rad_vec.norm();
    const RealVectorValue disp_vec(
        (*_disp_vals[0])[_qp], (*_disp_vals[1])[_qp], (_ndisp == 3 ? (*_disp_vals[2])[_qp] : 0.0));
    if (rad > 0.0)
    {
      rad_vec /= rad;
      rad_disp = rad_vec * disp_vec;
    }
    else
      rad_disp = disp_vec.norm();
  }
  else if (_coord_system == Moose::COORD_RSPHERICAL)
    rad_disp = (*_disp_vals[0])[_qp];
  else
    mooseError("Unsupported coordinate system");

  return rad_disp;
}
