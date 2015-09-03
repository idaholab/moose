/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MaxVarNDofsPerElemPP.h"

template<>
InputParameters validParams<MaxVarNDofsPerElemPP>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

MaxVarNDofsPerElemPP::MaxVarNDofsPerElemPP(const InputParameters & parameters) :
    GeneralPostprocessor(parameters)
{
}

PostprocessorValue
MaxVarNDofsPerElemPP::getValue()
{
  return _fe_problem.getNonlinearSystem().getMaxVarNDofsPerElem();
}
