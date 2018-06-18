//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TRACTIONSEPARATIONUOBASE_H
#define TRACTIONSEPARATIONUOBASE_H

#include "SideUserObject.h"
#include "RankTwoTensor.h"

class TractionSeparationUOBase;

template <>
InputParameters validParams<TractionSeparationUOBase>();

/**
Traction sepration law basic user object
 */
class TractionSeparationUOBase : public SideUserObject
{
public:
  TractionSeparationUOBase(const InputParameters & parameters);

  /// @{ Block all methods that are not used in explicitly called UOs
  virtual void initialize() override;
  virtual void execute() override final;
  virtual void finalize() override final;
  virtual void threadJoin(const UserObject &) override final;

  /// return a vector of strings containg stateful material properties names
  virtual void
  statefulMaterialPropertyNames(std::vector<std::string> & materialPropertyNames) const;

  /// --------------------------------------------------------
  /// methods that need to be overridden in chicld classes
  /// return the size of specific material property by its ID
  virtual unsigned int statefulMaterialPropertySize(unsigned int /*materialPropertyID*/) const;

  /// initialization of stateful material properties
  virtual void initStatefulMaterialProperty(unsigned int /*materialPropertyID*/,
                                            std::vector<Real> & /*statefulePropertyValue*/) const;

  /// method updating stateful material properties
  virtual void
  updateStatefulMaterialProperty(unsigned int /*qp*/,
                                 unsigned int /*materialPropertyID*/,
                                 std::vector<Real> & /*statefulePropertyValue*/,
                                 const std::vector<Real> & /*statefulePropertyValue_old*/) const;

  /// method returning the traction value in local coordinates
  virtual void computeTractionLocal(unsigned int qp, RealVectorValue & /*TractionLocal*/) const;
  /// method returning the traction derivates in local coordinates
  virtual void
  computeTractionSpatialDerivativeLocal(unsigned int /*qp*/,
                                        RankTwoTensor & /*TractionDerivativeLocal*/) const;

protected:
  /// The dispalcement jump accross the interface
  const MaterialProperty<RealVectorValue> & _JumpLocal;
  // the variable containing the list of the stateful
  // material properties variables
  const std::vector<std::string> _cohesive_law_stateful_properties_names;
};

#endif // TRACTIONSEPARATIONUOBASE_H
