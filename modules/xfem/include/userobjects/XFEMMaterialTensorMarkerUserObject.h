/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMMATERIALTENSORMARKERUSEROBJECT_H
#define XFEMMATERIALTENSORMARKERUSEROBJECT_H

#include "XFEMMarkerUserObject.h"
#include "MaterialTensorCalculator.h"

class XFEMMaterialTensorMarkerUserObject;

template <>
InputParameters validParams<XFEMMaterialTensorMarkerUserObject>();

class XFEMMaterialTensorMarkerUserObject : public XFEMMarkerUserObject
{
public:
  XFEMMaterialTensorMarkerUserObject(const InputParameters & parameters);
  virtual ~XFEMMaterialTensorMarkerUserObject() {}

protected:
  MaterialTensorCalculator _material_tensor_calculator;
  const MaterialProperty<SymmTensor> & _tensor;
  Real _threshold;
  bool _average;
  Real _random_range;

  virtual bool doesElementCrack(RealVectorValue & direction);
};

#endif // XFEMMATERIALTENSORMARKERUSEROBJECT_H
