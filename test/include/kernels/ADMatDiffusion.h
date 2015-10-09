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
#ifndef ADMATDIFFUSION_H
#define ADMATDIFFUSION_H

#include "ADKernel.h"
#include "MaterialProperty.h"

// Forward Declaration
class ADMatDiffusion;

template<>
InputParameters validParams<ADMatDiffusion>();


class ADMatDiffusion : public ADKernel
{
public:
  ADMatDiffusion(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  std::string _prop_name;
  const MaterialProperty<ADReal> * _diff;
};

#endif //ADMATDIFFUSION_H
