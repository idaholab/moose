//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "kEpsilonLinearVariable.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", kEpsilonLinearVariable);

InputParameters
kEpsilonLinearVariable::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates the linearization parameter (epsilon/k) in the k-epsilon model.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("epsilon",
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("C_mu", "Coupled turbulent kinetic energy closure.");
  params.addRequiredParam<std::vector<BoundaryName>>("walls",
                                                     "Boundaries that correspond to solid walls.");
  params.addParam<bool>(
      "linearized_yplus",
      false,
      "Boolean to indicate if yplus must be estimate locally for the blending functions.");

  params.addParam<Real>("max_mixing_length",
                        10.0,
                        "Maximum mixing legth allowed for the domain - adjust if seeking for "
                        "realizable k-epsilon answer.");
  return params;
}

kEpsilonLinearVariable::kEpsilonLinearVariable(const InputParameters & params)
  : AuxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _k(getFunctor<ADReal>("k")),
    _epsilon(getFunctor<ADReal>("epsilon")),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>("mu")),
    _C_mu(getFunctor<ADReal>("C_mu")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _linearized_yplus(getParam<bool>("linearized_yplus")),
    _max_mixing_length(getParam<Real>("max_mixing_length"))
{
}

Real
kEpsilonLinearVariable::computeValue()
{

  // Boundary value

  const Elem & elem = *_current_elem;

  bool wall_bounded = false;
  Real min_wall_dist = 0.0;
  Point loc_normal;

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
          Real dist = std::abs((fi->elemCentroid() - fi->faceCentroid()) * fi->normal());

          if (dist > min_wall_dist)
          {
            min_wall_dist = dist;
            loc_normal = fi->normal();
          }
          wall_bounded = true;
        }
      }
    }
  }

  if (false) //(wall_bounded)
  {
    // return wall bounded (epsilon/k) value

    constexpr Real karman_cte = 0.4187;
    constexpr Real E = 9.793;

    // Getting y_plus
    ADRealVectorValue velocity(_u_var->getElemValue(&elem));
    if (_v_var)
      velocity(1) = _v_var->getElemValue(&elem);
    if (_w_var)
      velocity(2) = _w_var->getElemValue(&elem);

    // Compute the velocity and direction of the velocity component that is parallel to the wall
    ADReal parallel_speed = (velocity - velocity * loc_normal * loc_normal).norm();

    ADReal u_star;
    if (_linearized_yplus)
    {
      const ADReal a_c = 1 / karman_cte;
      const ADReal b_c =
          1 / karman_cte * (std::log(E * min_wall_dist / _mu(makeElemArg(_current_elem))) + 1.0);
      const ADReal c_c = parallel_speed;
      u_star = (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c);
    }
    else
      u_star = NS::findUStar(
          _mu(makeElemArg(&elem)), _rho(makeElemArg(_current_elem)), parallel_speed, min_wall_dist);

    return (karman_cte * min_wall_dist / (u_star * std::sqrt(_C_mu(makeElemArg(_current_elem)))))
        .value();
  }
  else
  {
    //  Return Bulk value
    constexpr Real protection_k = 1e-15;
    auto current_argument = makeElemArg(_current_elem);
    // auto limiting_value = _C_mu(current_argument) * std::sqrt(_k(current_argument)) /
    // _max_mixing_length; // Realizable constraint

    // auto residual = std::max(_epsilon(current_argument) / (_k(current_argument) + protection_k),
    //                          limiting_value);

    auto k_value = std::max(_k(makeElemArg(_current_elem)), 1e-10);
    auto epsilon_value = std::max(_epsilon(makeElemArg(_current_elem)), 1e-10);

    auto residual =
        std::min(_epsilon(current_argument) / (_k(current_argument) + protection_k), 1e6);

    return residual.value();
  }
}
