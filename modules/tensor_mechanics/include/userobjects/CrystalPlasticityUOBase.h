/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRYSTALPLASTICITYUOBASE_H
#define CRYSTALPLASTICITYUOBASE_H

#include "DiscreteElementUserObject.h"

class CrystalPlasticityUOBase;

template <>
InputParameters validParams<CrystalPlasticityUOBase>();

/**
 * Crystal plasticity system userobject base class.
 */
class CrystalPlasticityUOBase : public DiscreteElementUserObject
{
public:
  CrystalPlasticityUOBase(const InputParameters & parameters);

  /// Returns the size of variable
  virtual unsigned int variableSize() const;

protected:
  unsigned int _variable_size;
};

#endif // CRYSTALPLASTICITYUOBASE_H
