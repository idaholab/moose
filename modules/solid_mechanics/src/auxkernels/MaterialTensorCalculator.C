//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialTensorCalculatorTools.h"
#include "MaterialTensorCalculator.h"

template <>
InputParameters
validParams<MaterialTensorCalculator>()
{
  InputParameters params = emptyInputParameters();
  MooseEnum quantities("VonMises=1 EffectiveStrain Hydrostatic Direction Hoop Radial Axial "
                       "MaxPrincipal MedPrincipal MinPrincipal FirstInvariant SecondInvariant "
                       "ThirdInvariant TriAxiality VolumetricStrain");

  params.addParam<int>(
      "index", -1, "The index into the tensor, from 0 to 5 (xx, yy, zz, xy, yz, zx).");
  params.addParam<MooseEnum>(
      "quantity", quantities, "A scalar quantity to compute: " + quantities.getRawNames());

  params.addParam<RealVectorValue>(
      "point1",
      RealVectorValue(0, 0, 0),
      "Start point for axis used to calculate some material tensor quantities");
  params.addParam<RealVectorValue>(
      "point2",
      RealVectorValue(0, 1, 0),
      "End point for axis used to calculate some material tensor quantities");
  params.addParam<RealVectorValue>("direction", RealVectorValue(1, 0, 0), "Direction vector");
  return params;
}

MaterialTensorCalculator::MaterialTensorCalculator(const InputParameters & parameters)
  : _index(parameters.get<int>("index")),
    _quantity_moose_enum(parameters.get<MooseEnum>("quantity")),
    _p1(parameters.get<RealVectorValue>("point1")),
    _p2(parameters.get<RealVectorValue>("point2")),
    _direction(parameters.get<RealVectorValue>("direction") /
               parameters.get<RealVectorValue>("direction").norm())
{
  const std::string & name = parameters.get<std::string>("_object_name");

  if (_quantity_moose_enum.isValid())
  {
    if (_index > 0)
      mooseError("Cannot define an index and a quantity in " + name);
    else
      _quantity = QUANTITY_ENUM(int(_quantity_moose_enum));
  }
  else
  {
    if (_index < 0)
      mooseError("Neither an index nor a quantity listed for " + name);
    else
      _quantity = COMPONENT; // default
  }

  if (_index > -1 && _index > 5)
  {
    mooseError("The material tensor index must be >= 0 and <= 5 OR < 0 (off).");
  }
}

Real
MaterialTensorCalculator::getTensorQuantity(const SymmTensor & tensor,
                                            const Point & curr_point,
                                            RealVectorValue & direction)
{
  direction.zero();
  Real value = 0.0;

  switch (_quantity)
  {
    case 0:
      value = MaterialTensorCalculatorTools::component(tensor, _index, direction);
      break;

    case 1:
      value = MaterialTensorCalculatorTools::vonMisesStress(tensor);
      break;

    case 2:
      value = MaterialTensorCalculatorTools::effectiveStrain(tensor);
      break;

    case 3:
      value = MaterialTensorCalculatorTools::hydrostatic(tensor);
      break;

    case 4:
      value = MaterialTensorCalculatorTools::directionValueTensor(tensor, _direction);
      break;

    case 5:
      value = MaterialTensorCalculatorTools::hoopStress(tensor, _p1, _p2, curr_point, direction);
      break;

    case 6:
      value = MaterialTensorCalculatorTools::radialStress(tensor, _p1, _p2, curr_point, direction);
      break;

    case 7:
      value = MaterialTensorCalculatorTools::axialStress(tensor, _p1, _p2, direction);
      break;

    case 8:
      value = MaterialTensorCalculatorTools::maxPrincipal(tensor, direction);
      break;

    case 9:
      value = MaterialTensorCalculatorTools::midPrincipal(tensor, direction);
      break;

    case 10:
      value = MaterialTensorCalculatorTools::minPrincipal(tensor, direction);
      break;

    case 11:
      value = MaterialTensorCalculatorTools::firstInvariant(tensor);
      break;

    case 12:
      value = MaterialTensorCalculatorTools::secondInvariant(tensor);
      break;

    case 13:
      value = MaterialTensorCalculatorTools::thirdInvariant(tensor);
      break;

    case 14:
      value = MaterialTensorCalculatorTools::triaxialityStress(tensor);
      break;

    case 15:
      value = MaterialTensorCalculatorTools::volumetricStrain(tensor);
      break;

    default:
      mooseError("Unknown quantity in MaterialTensorAux: " +
                 _quantity_moose_enum.operator std::string());
  }
  return value;
}
