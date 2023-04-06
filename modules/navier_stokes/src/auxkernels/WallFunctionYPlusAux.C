//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WallFunctionYPlusAux.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", WallFunctionYPlusAux);

InputParameters
WallFunctionYPlusAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates y+ value according to the algebraic velocity standard wall function.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("rho", "fluid density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity");
  params.addRequiredParam<std::vector<BoundaryName>>("walls",
                                                     "Boundaries that correspond to solid walls");
  return params;
}

WallFunctionYPlusAux::WallFunctionYPlusAux(const InputParameters & params)
  : AuxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _rho(getFunctor<ADReal>("rho")),
    _mu(getFunctor<ADReal>("mu")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls"))
{
  if (!_u_var)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !params.isParamValid("w"))
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");
}

Real
WallFunctionYPlusAux::computeValue()
{
  const Elem & elem = *_current_elem;

  bool wall_bounded = false;
  Real min_wall_dist = 1e10;
  Point wall_vec;
  Point normal;
  for (unsigned int i_side = 0; i_side < elem.n_sides(); ++i_side)
  {
    const std::vector<BoundaryID> side_bnds =
        _subproblem.mesh().getBoundaryIDs(_current_elem, i_side);
    for (const BoundaryName & name : _wall_boundary_names)
    {
      BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
      for (BoundaryID side_id : side_bnds)
      {
        if (side_id == wall_id)
        {
          const FaceInfo * const fi = _mesh.faceInfo(&elem, i_side);
          const Point & this_normal = fi->normal();
          Point this_wall_vec = (elem.vertex_average() - fi->faceCentroid());
          Real dist = std::abs(this_wall_vec * normal);
          if (dist < min_wall_dist)
          {
            min_wall_dist = dist;
            wall_vec = this_wall_vec;
            normal = this_normal;
          }
          wall_bounded = true;
        }
      }
    }
  }

  if (!wall_bounded)
    return 0;

  const auto state = determineState();

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var->getElemValue(&elem, state));
  if (_v_var)
    velocity(1) = _v_var->getElemValue(&elem, state);
  if (_w_var)
    velocity(2) = _w_var->getElemValue(&elem, state);

  // Compute the velocity and direction of the velocity component that is parallel to the wall
  Real dist = std::abs(wall_vec * normal);
  ADReal perpendicular_speed = velocity * normal;
  ADRealVectorValue parallel_velocity = velocity - perpendicular_speed * normal;
  ADReal parallel_speed = parallel_velocity.norm();
  ADRealVectorValue parallel_dir = parallel_velocity / parallel_speed;

  if (parallel_speed.value() < 1e-6)
    return 0;

  if (!std::isfinite(parallel_speed.value()))
    return parallel_speed.value();

  // Compute the friction velocity and the wall shear stress
  const auto elem_arg = makeElemArg(_current_elem);
  const auto rho = _rho(elem_arg, state);
  const auto mu = _mu(elem_arg, state);
  ADReal u_star = NS::findUStar(mu.value(), rho.value(), parallel_speed, dist);
  ADReal tau = u_star * u_star * rho;

  return (dist * u_star * rho / mu).value();
}
