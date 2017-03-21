/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GENERALIZEDPLANESTRAINACTION_H
#define GENERALIZEDPLANESTRAINACTION_H

#include "Action.h"

class GeneralizedPlaneStrainAction;

template <>
InputParameters validParams<GeneralizedPlaneStrainAction>();

class GeneralizedPlaneStrainAction : public Action
{
public:
  GeneralizedPlaneStrainAction(const InputParameters & params);

  void act() override;

protected:
  std::vector<NonlinearVariableName> _displacements;
  unsigned int _ndisp;
};
#endif // GENERALIZEDPLANESTRAINACTION_H
