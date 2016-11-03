/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSACTION_H
#define TENSORMECHANICSACTION_H

#include "Action.h"

class TensorMechanicsAction;

template<>
InputParameters validParams<TensorMechanicsAction>();

class TensorMechanicsAction : public Action
{
public:
  TensorMechanicsAction(const InputParameters & params);

  virtual void act();

protected:
  virtual std::string getKernelType();
  virtual InputParameters getParameters(std::string type);

  std::vector<NonlinearVariableName> _displacements;
  unsigned int _ndisp;

  std::vector<VariableName> _coupled_displacements;

  std::vector<AuxVariableName> _save_in;
  std::vector<AuxVariableName> _diag_save_in;

  Moose::CoordinateSystemType _coord_system;
};

#endif //TENSORMECHANICSACTION_H
