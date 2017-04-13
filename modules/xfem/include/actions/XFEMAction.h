/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMACTION_H
#define XFEMACTION_H

#include "Action.h"

class XFEMAction;

template <>
InputParameters validParams<XFEMAction>();

class XFEMAction : public Action
{
public:
  XFEMAction(InputParameters params);

  virtual void act();

protected:
  std::string _xfem_cut_type;
  std::vector<Real> _xfem_cut_data;
  std::vector<Real> _xfem_cut_scale;
  std::vector<Real> _xfem_cut_translate;
  std::string _xfem_qrule;
  std::string _order;
  std::string _family;
  bool _xfem_cut_plane;
  bool _xfem_use_crack_growth_increment;
  Real _xfem_crack_growth_increment;
};

#endif // XFEMACTION_H
