//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

class ElementOrderConversionGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  ElementOrderConversionGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

  enum class OrderConversionType
  {
    FIRST_ORDER,
    SECOND_ORDER_NONFULL,
    SECOND_ORDER,
    COMPLETE_ORDER
  };

protected:
  ///The input mesh
  std::unique_ptr<MeshBase> & _input;

  /// Type of Element Order Conversion
  const OrderConversionType _conversion_type;
};
