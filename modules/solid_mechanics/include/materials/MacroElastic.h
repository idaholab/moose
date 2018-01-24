/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MACROELASTIC_H
#define MACROELASTIC_H

#include "Elastic.h"

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

template <>
InputParameters validParams<Elastic>();

#endif
