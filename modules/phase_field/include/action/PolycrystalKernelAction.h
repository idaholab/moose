/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALKERNELACTION_H
#define POLYCRYSTALKERNELACTION_H

#include "Action.h"

/**
 * Action that sets up ACGrGrPoly, ACInterface, TimeDerivative, and ACGBPoly
 * kernels.
 */
class PolycrystalKernelAction : public Action
{
public:
  PolycrystalKernelAction(const InputParameters & params);

  void act() override;

protected:
  virtual void setupACBulkKernel(InputParameters params, std::string var_name);
  virtual std::string getACBulkName() { return "ACGrGrPoly"; }

  /// number of grains to create
  const unsigned int _op_num;

  /// current order parameter being processed
  unsigned int _op;

  /// base name for the order parameter variables
  const std::string _var_name_base;
};

template <>
InputParameters validParams<PolycrystalKernelAction>();

#endif // POLYCRYSTALKERNELACTION_H
