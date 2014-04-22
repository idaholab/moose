/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALTENSORCALCULATOR_H
#define MATERIALTENSORCALCULATOR_H

#include "libmesh/vector_value.h"
#include "InputParameters.h"
#include "SymmTensor.h"

class MaterialTensorCalculator;

template<>
InputParameters validParams<MaterialTensorCalculator>();

class MaterialTensorCalculator
{
public:

  enum QUANTITY_ENUM
  {
    COMPONENT,
    VONMISES,
    PLASTICSTRAINMAG,
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

  MaterialTensorCalculator(const std::string &name, InputParameters parameters);
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
                         const Point * curr_point,
                         RealVectorValue &direction);

  Real principalValue( const SymmTensor & tensor, unsigned int index, RealVectorValue &direction );

};

#endif //MATERIALTENSORCALCULATOR_H
