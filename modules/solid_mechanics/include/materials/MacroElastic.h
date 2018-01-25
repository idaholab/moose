//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MACROELASTIC_H
#define MACROELASTIC_H

#include "Elastic.h"

class MacroElastic;

template <>
InputParameters validParams<MacroElastic>();

class MacroElastic : public Elastic
{
public:
  MacroElastic(const InputParameters & parameters);
  virtual ~MacroElastic();

protected:
  virtual void createElasticityTensor();

  virtual bool updateElasticityTensor(SymmElasticityTensor & tensor);

  virtual void checkElasticConstants() {}

private:
  const PostprocessorValue & _C1111;
  const PostprocessorValue & _C1122;
  const PostprocessorValue & _C1133;
  const PostprocessorValue & _C2222;
  const PostprocessorValue & _C2233;
  const PostprocessorValue & _C3333;
  const PostprocessorValue & _C1212;
  const PostprocessorValue & _C2323;
  const PostprocessorValue & _C3131;
};

#endif
