//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LikelihoodInterface.h"

InputParameters
LikelihoodInterface::validParams()
{
  return emptyInputParameters();
}

LikelihoodInterface::LikelihoodInterface(const InputParameters & parameters)
  : _likelihood_feproblem(*parameters.get<FEProblemBase *>("_fe_problem_base"))
{
}

LikelihoodFunctionBase *
LikelihoodInterface::getLikelihoodFunctionByName(const UserObjectName & name) const
{
  std::vector<LikelihoodFunctionBase *> models;
  _likelihood_feproblem.theWarehouse()
      .query()
      .condition<AttribName>(name)
      .condition<AttribSystem>("LikelihoodFunctionBase")
      .queryInto(models);

  if (models.empty())
    mooseError("Unable to find a LikelihoodFunction object with the name '" + name + "'");
  return models[0];
}

LikelihoodFunctionBaseVector *
LikelihoodInterface::getLikelihoodVectorFunctionByName(const UserObjectName & name) const
{
  std::vector<LikelihoodFunctionBaseVector *> models;
  _likelihood_feproblem.theWarehouse()
      .query()
      .condition<AttribName>(name)
      .condition<AttribSystem>("LikelihoodFunctionBaseVector")
      .queryInto(models);

  if (models.empty())
    mooseError("Unable to find a LikelihoodFunctionVector object with the name '" + name + "'");
  return models[0];
}

LikelihoodFunctionTypes
LikelihoodInterface::queryLikelihoodFunctionType(const UserObjectName & name)
{
  LikelihoodFunctionTypes result;
  std::vector<LikelihoodFunctionBase *> models;
  _likelihood_feproblem.theWarehouse()
      .query()
      .condition<AttribName>(name)
      .condition<AttribSystem>("LikelihoodFunctionBase")
      .queryInto(models);

  if (models.empty())
  {
    std::vector<LikelihoodFunctionBaseVector *> v_models;
    _likelihood_feproblem.theWarehouse()
        .query()
        .condition<AttribName>(name)
        .condition<AttribSystem>("LikelihoodFunctionBaseVector")
        .queryInto(v_models);
    if (v_models.empty())
    {
      std::vector<MooseObject *> param_result;
      _likelihood_feproblem.theWarehouse().query().condition<AttribName>(name).queryInto(
          param_result);
      std::vector<MooseObject *> param_result_2;
      _likelihood_feproblem.theWarehouse()
          .query()
          .condition<AttribSystem>("LikelihoodFunctionBase")
          .queryInto(param_result_2);
      std::vector<MooseObject *> param_result_3;
      _likelihood_feproblem.theWarehouse()
          .query()
          .condition<AttribSystem>("LikelihoodFunctionBaseVector")
          .queryInto(param_result_3);
      std::vector<MooseObject *> param_result_4;
      _likelihood_feproblem.theWarehouse().query().queryInto(param_result_4);

      std::cout << "Couldn't find " << name << " in objects. ";
      for (const auto & obj : param_result)
        std::cout << obj->typeAndName() << std::endl;
      std::cout << std::endl << std::endl << "LikelihoodFunctionBase objects" << std::endl;
      for (const auto & obj : param_result_2)
        std::cout << obj->typeAndName() << std::endl;
      std::cout << std::endl << std::endl << "LikelihoodFunctionBaseVector objects" << std::endl;
      for (const auto & obj : param_result_3)
        std::cout << obj->typeAndName() << std::endl;
      std::cout << std::endl << std::endl << "All objects" << std::endl;
      for (const auto & obj : param_result_4)
        std::cout << obj->typeAndName() << std::endl;
      mooseError("Unable to find a LikelihoodFunction or LikelihoodFunctionVector "
                 "object with the name '" +
                 name + "'");
    }
    else
      result = LikelihoodFunctionTypes::VECTOR;
  }
  else
  {
    result = LikelihoodFunctionTypes::SCALAR;
  }
  return result;
}
