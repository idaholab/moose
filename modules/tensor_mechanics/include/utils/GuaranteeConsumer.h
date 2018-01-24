/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GUARANTEECONSUMER_H
#define GUARANTEECONSUMER_H

#include "Guarantee.h"
#include "MooseTypes.h"

class MooseObject;
class FEProblemBase;
class InputParameters;
class BlockRestrictable;

/**
 * Add-on class that provides the functionality to check if guarantees for
 * material properties are provided. The types of guarantees are listed in
 * Guarantees.h
 */
class GuaranteeConsumer
{
public:
  GuaranteeConsumer(MooseObject * moose_object);

protected:
  bool hasGuaranteedMaterialProperty(const MaterialPropertyName & prop, Guarantee guarantee);

private:
  /// Parameters of the object with this interface
  const InputParameters & _gc_params;

  /// Reference to the FEProblemBase class
  FEProblemBase * const _gc_feproblem;

  /// Access block restrictions of the object with this interface
  BlockRestrictable * const _gc_block_restrict;
};

#endif // GUARANTEECONSUMER_H
