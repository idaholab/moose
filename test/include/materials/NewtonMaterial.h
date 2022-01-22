//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Material.h"

/**
 * A test object that uses Material to perform a Newton solve of a material property.
 *
 * Also, does some error checking.
 */
class NewtonMaterial : public Material
{
public:
  static InputParameters validParams();

  NewtonMaterial(const InputParameters & parameters);
  virtual ~NewtonMaterial(){};

  virtual void initialSetup() override;

protected:
  void computeQpProperties() override;

private:
  const Real & _tol;
  const MaterialProperty<Real> & _f;
  const MaterialProperty<Real> & _f_prime;
  MaterialProperty<Real> & _p;
  std::vector<unsigned int> _prop_ids;
  unsigned int _max_iterations;
  MaterialBase * _discrete;
};
