/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMMATERIALMANAGERDIRACKERNEL_H
#define XFEMMATERIALMANAGERDIRACKERNEL_H

#include "DiracKernel.h"
#include "XFEMMaterialManager.h"

/**
 * DiracKernel that utilizes the XFEMMaterialManager
 */
class XFEMMaterialManagerDiracKernel : public DiracKernel
{
public:
  XFEMMaterialManagerDiracKernel(const InputParameters & parameters);

  virtual void addPoints() final;

  virtual void computeResidual();
  virtual void computeJacobian();

  template <typename T>
  const MaterialProperty<T> * getMaterialProperty(const std::string & name) const;
  template <typename T>
  const MaterialProperty<T> * getMaterialPropertyOld(const std::string & name) const;
  template <typename T>
  const MaterialProperty<T> * getMaterialPropertyOlder(const std::string & name) const;

protected:
  const XFEMMaterialManager & _manager;
};

template <>
InputParameters validParams<XFEMMaterialManagerDiracKernel>();

template <typename T>
const MaterialProperty<T> *
XFEMMaterialManagerDiracKernel::getMaterialProperty(const std::string & name) const
{
  return &_manager.getMaterialProperty<T>(name);
}

template <typename T>
const MaterialProperty<T> *
XFEMMaterialManagerDiracKernel::getMaterialPropertyOld(const std::string & name) const
{
  return &_manager.getMaterialPropertyOld<T>(name);
}

template <typename T>
const MaterialProperty<T> *
XFEMMaterialManagerDiracKernel::getMaterialPropertyOlder(const std::string & name) const
{
  return &_manager.getMaterialPropertyOlder<T>(name);
}

#endif // XFEMMATERIALMANAGERDIRACKERNEL_H
