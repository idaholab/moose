/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATERIALVECTORAUXKERNELACTION_H
#define MATERIALVECTORAUXKERNELACTION_H

#include "Action.h"

class MaterialVectorAuxKernelAction : public Action
{
public:
  MaterialVectorAuxKernelAction(const InputParameters & params);

  virtual void act();

protected:
  /// number of grains to create
  const unsigned int _grain_num;

  /// base name for the auxvariables
  const std::vector<std::string> & _var_name_base;

  /// number of auxvariables
  const unsigned int _num_var;

  /// list of material properties to be used
  const std::vector<MaterialPropertyName> & _prop;

  /// number of properties
  const unsigned int _num_prop;
};

template <>
InputParameters validParams<MaterialVectorAuxKernelAction>();

#endif // MATERIALVECTORAUXKERNELACTION_H
