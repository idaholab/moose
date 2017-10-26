/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMMATERIALMANAGERCONSTRAINT_H
#define XFEMMATERIALMANAGERCONSTRAINT_H

#include "ElemElemConstraint.h"
#include "XFEMElemPairMaterialManager.h"

/**
 * ElemElemConstraint that utilizes the XFEMMaterialManager
 */
class XFEMMaterialManagerConstraint : public ElemElemConstraint
{
public:
  XFEMMaterialManagerConstraint(const InputParameters & parameters);

  virtual void computeResidual();
  virtual void computeJacobian();

  template <typename T>
  const MaterialProperty<T> * getMaterialProperty(const std::string & name) const;
  template <typename T>
  const MaterialProperty<T> * getMaterialPropertyOld(const std::string & name) const;
  template <typename T>
  const MaterialProperty<T> * getMaterialPropertyOlder(const std::string & name) const;

protected:
  const XFEMElemPairMaterialManager & _manager;
};

template <>
InputParameters validParams<XFEMMaterialManagerConstraint>();

template <typename T>
const MaterialProperty<T> *
XFEMMaterialManagerConstraint::getMaterialProperty(const std::string & name) const
{
  return &_manager.getMaterialProperty<T>(name);
}

template <typename T>
const MaterialProperty<T> *
XFEMMaterialManagerConstraint::getMaterialPropertyOld(const std::string & name) const
{
  return &_manager.getMaterialPropertyOld<T>(name);
}

template <typename T>
const MaterialProperty<T> *
XFEMMaterialManagerConstraint::getMaterialPropertyOlder(const std::string & name) const
{
  return &_manager.getMaterialPropertyOlder<T>(name);
}

#endif // XFEMMATERIALMANAGERCONSTRAINT_H
