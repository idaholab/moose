//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TangentialMortarLMMechanicalContact.h"
#include "SubProblem.h"

registerMooseObject("MooseApp", TangentialMortarLMMechanicalContact);

InputParameters
TangentialMortarLMMechanicalContact::validParams()
{
  auto params = MortarConstraintBase::validParams();
  params.addRequiredParam<NonlinearVariableName>("secondary_disp_y",
                                                 "The y displacement variable on the secondary face");
  params.addParam<NonlinearVariableName>("primary_disp_y",
                                         "The y displacement variable on the primary face");
  params.addRequiredParam<NonlinearVariableName>(
      "contact_pressure",
      "The normal contact pressure; oftentimes this may be a separate lagrange multiplier "
      "variable");
  params.addRequiredParam<Real>("friction_coefficient", "The friction coefficient");

  MooseEnum ncp_type("min fb", "fb");
  params.addParam<MooseEnum>("ncp_function_type",
                             ncp_type,
                             "The type of the nonlinear complimentarity function; options are "
                             "min or fb where fb stands for Fischer-Burmeister");
  params.addParam<Real>(
      "c", 1, "Parameter for balancing the size of the velocity and the pressures");
  params.set<bool>("compute_primal_residuals") = false;
  params.addClassDescription("Ensures that the Karush-Kuhn-Tucker conditions of Coulomb "
                             "frictional contact are satisfied");
  return params;
}

TangentialMortarLMMechanicalContact::TangentialMortarLMMechanicalContact(
    const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _secondary_disp_y(
        this->_subproblem.getStandardVariable(_tid, parameters.getMooseType("secondary_disp_y"))),
    _primary_disp_y(
        isParamValid("primary_disp_y")
            ? this->_subproblem.getStandardVariable(_tid, parameters.getMooseType("primary_disp_y"))
            : this->_subproblem.getStandardVariable(_tid, parameters.getMooseType("secondary_disp_y"))),
    _contact_pressure_var(
        this->_subproblem.getStandardVariable(_tid, parameters.getMooseType("contact_pressure"))),
    _contact_pressure(_contact_pressure_var.adSlnLower()),
    _secondary_x_dot(_secondary_var.adUDot()),
    _primary_x_dot(_primary_var.adUDotNeighbor()),
    _secondary_y_dot(_secondary_disp_y.adUDot()),
    _primary_y_dot(_primary_disp_y.adUDotNeighbor()),
    _friction_coeff(getParam<Real>("friction_coefficient")),
    _epsilon(std::numeric_limits<Real>::epsilon()),
    _ncp_type(getParam<MooseEnum>("ncp_function_type")),
    _c(getParam<Real>("c"))
{
}

ADReal
TangentialMortarLMMechanicalContact::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Lower:
    {
      // Check whether we project onto a primary face
      if (_has_primary)
      {
        // Check whether we are actually in contact
        if (_contact_pressure[_qp] > TOLERANCE * TOLERANCE)
        {
          // Build the velocity vector
          ADRealVectorValue relative_velocity(
              _secondary_x_dot[_qp] - _primary_x_dot[_qp], _secondary_y_dot[_qp] - _primary_y_dot[_qp], 0);

          // Get the component in the tangential direction
          auto tangential_velocity = relative_velocity * _tangents[_qp][0];

          // NCP part 1: requirement that either there is no slip **or** slip velocity and
          // frictional force exerted **by** the secondary side are in the same direction
          ADReal a;
          if (tangential_velocity * _lambda[_qp] < 0)
            a = -std::abs(tangential_velocity);
          else
          {
            if (tangential_velocity == 0)
              // Avoid a singular Jacobian entry
              tangential_velocity += _epsilon;
            a = std::abs(tangential_velocity);
          }
          a *= _c;

          // NCP part 2: require that the frictional force can never exceed the frictional
          // coefficient times the normal force
          auto b = _friction_coeff * _contact_pressure[_qp] - std::abs(_lambda[_qp]);

          ADReal fb_function;
          if (_ncp_type == "fb")
            // The FB function (in its pure form) is not differentiable at (0, 0) but if we add some
            // constant > 0 into the root function, then it is
            fb_function = a + b - std::sqrt(a * a + b * b + _epsilon);
          else
            fb_function = std::min(a, b);

          return _test[_i][_qp] * fb_function;
        }
        else
          // If not in contact then we force the tangential lagrange multiplier to zero
          return _test[_i][_qp] * _lambda[_qp];
      }
      else
        // If not in contact then we force the tangential lagrange multiplier to zero (if we don't
        // project onto a primary face, then we're definitely not in contact)
        return _test[_i][_qp] * _lambda[_qp];
    }

    default:
      return 0;
  }
}
