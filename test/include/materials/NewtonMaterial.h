//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Material.h"
#include "NestedSolve.h"

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
  const MaterialProperty<Real> & _f;
  const MaterialProperty<Real> & _f_prime;
  MaterialProperty<Real> & _p;
  std::vector<unsigned int> _prop_ids;
  MaterialBase * _discrete;
  NestedSolve _nested_solve;
};
