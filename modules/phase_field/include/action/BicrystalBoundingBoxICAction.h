/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef BICRYSTALBOUNDINGBOXICACTION_H
#define BICRYSTALBOUNDINGBOXICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Bicrystal using a bounding box
 */
class BicrystalBoundingBoxICAction : public Action
{
public:
  BicrystalBoundingBoxICAction(const InputParameters & params);

  virtual void act();

private:
  const std::string _var_name_base;
  const unsigned int _op_num;
};

template <>
InputParameters validParams<BicrystalBoundingBoxICAction>();

#endif // BICRYSTALBOUNDINGBOXICACTION_H
