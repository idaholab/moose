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
#include "PercentChangePostprocessor.h"

template<>
InputParameters validParams<PercentChangePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor used for exit criterion");
  return params;
}

PercentChangePostprocessor::PercentChangePostprocessor(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _postprocessor_old(getPostprocessorValueOld("postprocessor"))

{
}

PercentChangePostprocessor::~PercentChangePostprocessor(){
}

void
PercentChangePostprocessor::initialize(){
}

void
PercentChangePostprocessor::execute(){
}

Real
PercentChangePostprocessor::getValue()
{
  return fabs( ( fabs(_postprocessor)- fabs(_postprocessor_old) )*pow(fabs(_postprocessor),-1));
}
