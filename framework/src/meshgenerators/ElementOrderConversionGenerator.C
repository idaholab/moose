//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementOrderConversionGenerator.h"

#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/boundary_info.h"

registerMooseObject("MooseApp", ElementOrderConversionGenerator);

InputParameters
ElementOrderConversionGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum conversion_type("FIRST_ORDER SECOND_ORDER_NONFULL SECOND_ORDER COMPLETE_ORDER",
                            "FIRST_ORDER");

  params.addClassDescription("Mesh generator which converts orders of elements");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<MooseEnum>(
      "conversion_type", conversion_type, "The type of element order conversion to perform");

  return params;
}

ElementOrderConversionGenerator::ElementOrderConversionGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _conversion_type(getParam<MooseEnum>("conversion_type").template getEnum<OrderConversionType>())
{
}

std::unique_ptr<MeshBase>
ElementOrderConversionGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  switch (_conversion_type)
  {
    case OrderConversionType::FIRST_ORDER:
      mesh->all_first_order();
      break;
    case OrderConversionType::SECOND_ORDER_NONFULL:
      mesh->all_second_order(false);
      break;
    case OrderConversionType::SECOND_ORDER:
      mesh->all_second_order();
      break;
    case OrderConversionType::COMPLETE_ORDER:
      mesh->all_complete_order();
      break;
    default:
      mooseError("Invalid conversion type");
  }

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
