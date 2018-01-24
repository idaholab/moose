//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/*************************************************************************
*
*  Welcome to HYRAX!
*  Andrea M. Jokisaari
*  CASL/MOOSE
*
*  18 April 2012
*
*************************************************************************/

#ifndef MATERIALSYMMELASTICITYTENSORAUX_H
#define MATERIALSYMMELASTICITYTENSORAUX_H

#include "AuxKernel.h"
#include "SymmElasticityTensor.h"

// Forward declarations
class MaterialSymmElasticityTensorAux;
class SymmElasticityTensor;

template <>
InputParameters validParams<MaterialSymmElasticityTensorAux>();

class MaterialSymmElasticityTensorAux : public AuxKernel
{
public:
  MaterialSymmElasticityTensorAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  std::string _tensor_matpro;
  int _index;

private:
  const MaterialProperty<SymmElasticityTensor> & _tensor_prop;
};

#endif // MATERIALSYMMELASTICITYTENSORAUX_H
