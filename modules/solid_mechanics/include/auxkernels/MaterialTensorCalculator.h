//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALTENSORCALCULATOR_H
#define MATERIALTENSORCALCULATOR_H

// MOOSE includes
#include "InputParameters.h"
#include "MooseEnum.h"
#include "SymmTensor.h"

#include "libmesh/vector_value.h"

class MaterialTensorCalculator;

template <>
InputParameters validParams<MaterialTensorCalculator>();

class MaterialTensorCalculator
{
public:
  enum QUANTITY_ENUM
  {
    COMPONENT,
    VONMISES,
    EFFECTIVESTRAIN,
    HYDROSTATIC,
    DIRECTION,
    HOOP,
    RADIAL,
    AXIAL,
    MAXPRINCIPAL,
    MEDPRINCIPAL,
    MINPRINCIPAL,
    FIRSTINVARIANT,
    SECONDINVARIANT,
    THIRDINVARIANT,
    TRIAXIALITY,
    VOLUMETRICSTRAIN
  };

  MaterialTensorCalculator(const InputParameters & parameters);

  ~MaterialTensorCalculator() {}

protected:
  const int _index;
  MooseEnum _quantity_moose_enum;
  QUANTITY_ENUM _quantity;

  const Point _p1;
  const Point _p2;
  const Point _direction;

public:
  Real getTensorQuantity(const SymmTensor & tensor,
                         const Point & curr_point,
                         RealVectorValue & direction);
};

#endif // MATERIALTENSORCALCULATOR_H
