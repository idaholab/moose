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

#include "Stabilizer.h"
#include "MooseSystem.h"
#include "ElementData.h"

#include <vector>

template<>
InputParameters validParams<Stabilizer>()
{
  InputParameters params = validParams<PDEBase>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Stabilizer will act on.");
  return params;
}


Stabilizer::Stabilizer(const std::string & name, MooseSystem & moose_system, InputParameters parameters) :
  PDEBase(name, moose_system, parameters, *moose_system._element_data[parameters.get<THREAD_ID>("_tid")]),
  MaterialPropertyInterface(moose_system._material_data[_tid]),
  _element_data(*moose_system._element_data[_tid]),
  _test(_element_data._test[_var_num]),
  _grad_test(*_element_data._grad_phi[_fe_type])
{
}

Stabilizer::~Stabilizer()
{
}

Real
Stabilizer::computeQpResidual()
{
  return 0;
}
