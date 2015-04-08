/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDV_H
#define ADDV_H

#include "InputParameters.h"

InputParameters & AddV(InputParameters & parameters, const std::string & var_name = "v");

#endif //ADDV_H
