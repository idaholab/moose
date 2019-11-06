//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * Stateful material class that defines a few properties.
 */
class ImplicitStateful : public Material
{
public:
  static InputParameters validParams();

  ImplicitStateful(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

private:
  bool _add_time;
  bool _use_older;
  MaterialProperty<Real> & _prop;
  const MaterialProperty<Real> & _coupled_old;
  const MaterialProperty<Real> & _coupled_older;
};
