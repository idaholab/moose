//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SOLIDPROPERTIES_H
#define SOLIDPROPERTIES_H

#include "ThreadedGeneralUserObject.h"

class SolidProperties;

template <>
InputParameters validParams<SolidProperties>();

/**
 * Base class for defining solid property user objects.
 */
class SolidProperties : public ThreadedGeneralUserObject
{
public:
  SolidProperties(const InputParameters & parameters);

  virtual void execute() final {}
  virtual void initialize() final {}
  virtual void finalize() final {}

  virtual void threadJoin(const UserObject &) final {}
  virtual void subdomainSetup() final {}

  /**
   * Solid name
   * @return string representing solid name
   */
  virtual const std::string & solidName() const;

  /**
   * Molar mass
   * @return molar mass
   */
  virtual Real molarMass() const;

private:
  /// The solid name
  static const std::string _name;
};

#endif /* SOLIDPROPERTIES_H */
