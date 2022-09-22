//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputFileFormatter.h"

#include <sstream>
#include <string>
#include <vector>

class ActionWarehouse;

/*
 * Dumps the input file in the Exodus output
 */
class ExodusFormatter : public InputFileFormatter
{
public:
  ExodusFormatter();

  void printInputFile(ActionWarehouse & wh);

  void format();

  std::vector<std::string> & getInputFileRecord() { return _input_file_record; }

protected:
  std::stringstream _ss;
  std::vector<std::string> _input_file_record;
};
