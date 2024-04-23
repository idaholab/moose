//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEMaterialPropertyToNEML2.h"

registerMooseObject("SolidMechanicsApp", MOOSERealMaterialPropertyToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSERankTwoTensorMaterialPropertyToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSEStdVectorRealMaterialPropertyToNEML2);

#ifndef NEML2_ENABLED
#define MOOSEMaterialPropertyToNEML2Stub(name)                                                     \
  NEML2ObjectStubImplementationOpen(name, ElementUserObject);                                      \
  NEML2ObjectStubParam(MaterialPropertyName, "moose_material_property");                           \
  NEML2ObjectStubParam(std::string, "neml2_variable");                                             \
  NEML2ObjectStubImplementationClose(name, ElementUserObject)
MOOSEMaterialPropertyToNEML2Stub(MOOSERealMaterialPropertyToNEML2);
MOOSEMaterialPropertyToNEML2Stub(MOOSERankTwoTensorMaterialPropertyToNEML2);
MOOSEMaterialPropertyToNEML2Stub(MOOSEStdVectorRealMaterialPropertyToNEML2);
#else

template <typename T>
InputParameters
MOOSEMaterialPropertyToNEML2<T>::validParams()
{
  auto params = MOOSEToNEML2::validParams();
  params.addClassDescription("Gather a MOOSE material property of type " +
                             demangle(typeid(T).name()) +
                             " for insertion into the specified input of a "
                             "NEML2 model.");

  params.addRequiredParam<MaterialPropertyName>("moose_material_property",
                                                "MOOSE material property to read from");
  return params;
}

template <typename T>
MOOSEMaterialPropertyToNEML2<T>::MOOSEMaterialPropertyToNEML2(const InputParameters & params)
  : MOOSEToNEML2(params), _mat_prop(getMaterialProperty<T>("moose_material_property"))
{
}

template <typename T>
void
MOOSEMaterialPropertyToNEML2<T>::execute()
{
#ifdef NEML2_ENABLED
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    _buffer.push_back(NEML2Utils::toNEML2<T>(_mat_prop[qp]));
#endif
}

template class MOOSEMaterialPropertyToNEML2<Real>;
template class MOOSEMaterialPropertyToNEML2<RankTwoTensor>;
template class MOOSEMaterialPropertyToNEML2<std::vector<Real>>;

#endif
