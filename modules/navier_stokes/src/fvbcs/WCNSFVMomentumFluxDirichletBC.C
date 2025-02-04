//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVMomentumFluxDirichletBC.h"
#include "INSFVVelocityVariable.h"

registerMooseObject("NavierStokesApp", WCNSFVMomentumFluxDirichletBC);

InputParameters
WCNSFVMomentumFluxDirichletBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params += INSFVFlowBC::validParams();

  params.addClassDescription("Specified fixed inlet mass flow rate via postprocessor.");

  params.addParam<Real>("scaling_factor", 1, "To scale the mass flux");
  params.addParam<PostprocessorName>("mdot_pp", "Postprocessor with the inlet mass flow rate");
  params.addRequiredParam<PostprocessorName>("area_pp", "Inlet area as a postprocessor");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density functor");
  params.addParam<Point>(
      "direction",
      Point(),
      "The direction of the flow at the boundary. This is mainly used for cases when an inlet "
      "angle needs to be defined with respect to the normal and when a boundary is defined on an "
      "internal face where the normal can point in both directions. Use positive mass flux and "
      "velocity magnitude if the flux aligns with this direction vector.");

  return params;
}

WCNSFVMomentumFluxDirichletBC::WCNSFVMomentumFluxDirichletBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    INSFVFlowBC(params),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _mdot_pp(isParamValid("mdot_pp") ? &getPostprocessorValue("mdot_pp") : nullptr),
    _area_pp(isParamValid("area_pp") ? &getPostprocessorValue("area_pp") : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _direction(getParam<Point>("direction")),
    _direction_specified_by_user(params.isParamSetByUser("direction"))
{
  if (!dynamic_cast<INSFVVelocityVariable *>(&_var))
    paramError("variable",
               "The variable argument to WCNSFVMomentumFluxDirichletBC must be of type "
               "INSFVVelocityVariable");

  if (_direction_specified_by_user && !MooseUtils::absoluteFuzzyEqual(_direction.norm(), 1.0, 1e-6))
    paramError("direction", "The direction should be a unit vector with a tolerance of 1e-6!");
}

ADReal
WCNSFVMomentumFluxDirichletBC::boundaryValue(const FaceInfo & fi,
                                             const Moose::StateArg & state) const
{
  // Getting firection of the mass flow rate relative to the inlet
  const ADRealVectorValue incoming_vector = !_direction_specified_by_user ? _normal : _direction;
  const ADReal cos_angle = std::abs(incoming_vector * _normal);

  auto sfa = singleSidedFaceArg(&fi);
  const auto rho = _rho(sfa, state);
  return *_mdot_pp / rho / *_area_pp * cos_angle;
}
