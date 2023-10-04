//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContinuousGalerkin.h"
#include "MooseEnum.h"

registerMooseObject("MooseApp", ContinuousGalerkin);

InputParameters
ContinuousGalerkin::validParams()
{
  InputParameters params = PhysicsDiscretization::validParams();
  params.addClassDescription(
      "Use a continuous Galerkin finite element discretization for your physics");

  // Remove non-CGFE families
  // We'll allow scalar and vector CGFE for now, to make this class inclusive of other variable
  // types used in CGFE
  MooseEnum families("LAGRANGE HERMITE SCALAR HIERARCHIC CLOUGH SZABAB BERNSTEIN LAGRANGE_VEC "
                     "RATIONAL_BERNSTEIN",
                     "LAGRANGE");
  params.addParam<std::vector<MooseEnum>>(
      "family", {families}, "Specifies the family of FE shape functions to use for this variable");

  // TODO
  // This should not be necessary, as these params should already have been added
  params.set<std::string>("_name") = "ContinuousGalerkin";
  params.set<std::string>("_type") = "ContinuousGalerkin";
  params.addParam<std::string>("_object_name", "ContinuousGalerking");
  params.addParam<bool>("enable", true, "");
  params.addParam<std::vector<std::string>>("control_tags", {});

  return params;
}

ContinuousGalerkin::ContinuousGalerkin(const InputParameters & parameters)
  : PhysicsDiscretization(parameters)
{
}
