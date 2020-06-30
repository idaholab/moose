//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalMortarLMMechanicalContact.h"
#include "SubProblem.h"

registerMooseObject("ContactApp", NormalMortarLMMechanicalContact);

InputParameters
NormalMortarLMMechanicalContact::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription(
      "Enforces the normal contact complementarity conditions in a mortar discretization");
  params.addParam<NonlinearVariableName>("secondary_disp_y",
                                         "The y displacement variable on the secondary face");
  params.addParam<NonlinearVariableName>("primary_disp_y",
                                         "The y displacement variable on the primary face");
  MooseEnum ncp_type("min fb", "min");
  params.addParam<MooseEnum>("ncp_function_type",
                             ncp_type,
                             "The type of the nonlinear complimentarity function; options are "
                             "min or fb where fb stands for Fischer-Burmeister");
  return params;
}

NormalMortarLMMechanicalContact::NormalMortarLMMechanicalContact(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _secondary_disp_y(isParamValid("secondary_disp_y")
                          ? &this->_subproblem.getStandardVariable(
                                _tid, parameters.getMooseType("secondary_disp_y"))
                          : nullptr),
    _primary_disp_y(isParamValid("primary_disp_y")
                        ? &this->_subproblem.getStandardVariable(
                              _tid, parameters.getMooseType("primary_disp_y"))
                        : isParamValid("secondary_disp_y")
                              ? &this->_subproblem.getStandardVariable(
                                    _tid, parameters.getMooseType("secondary_disp_y"))
                              : nullptr),
    _computing_gap_dependence(false),
    _secondary_disp_y_sln(nullptr),
    _primary_disp_y_sln(nullptr),
    _epsilon(std::numeric_limits<Real>::epsilon()),
    _ncp_type(getParam<MooseEnum>("ncp_function_type"))
{
  if (_secondary_disp_y)
  {
    mooseAssert(
        _primary_disp_y,
        "It doesn't make any sense that we have a secondary displacement variable and not a "
        "primary displacement variable");
    _computing_gap_dependence = true;
    _secondary_disp_y_sln = &_secondary_disp_y->adSln();
    _primary_disp_y_sln = &_primary_disp_y->adSlnNeighbor();
  }
}

ADReal
NormalMortarLMMechanicalContact::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Lower:
    {
      if (_has_primary)
      {
        DualRealVectorValue gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];
        if (_computing_gap_dependence)
        {
          // Here we're assuming that the user provided the x-component as the secondary/primary
          // variable!
          gap_vec(0).derivatives() =
              _u_primary[_qp].derivatives() - _u_secondary[_qp].derivatives();
          gap_vec(1).derivatives() = (*_primary_disp_y_sln)[_qp].derivatives() -
                                     (*_secondary_disp_y_sln)[_qp].derivatives();
        }

        auto gap = gap_vec * _normals[_qp];

        const auto & a = _lambda[_qp];
        const auto & b = gap;

        DualReal fb_function;
        if (_ncp_type == "fb")
          // The FB function (in its pure form) is not differentiable at (0, 0) but if we add some
          // constant > 0 into the root function, then it is
          fb_function = a + b - std::sqrt(a * a + b * b + _epsilon);
        else
          fb_function = std::min(a, b);

        return _test[_i][_qp] * fb_function;
      }
      else
        return _test[_i][_qp] * _lambda[_qp];
    }

    default:
      return 0;
  }
}
