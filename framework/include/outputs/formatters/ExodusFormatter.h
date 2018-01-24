//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXODUSFORMATTER_H
#define EXODUSFORMATTER_H

#include "InputFileFormatter.h"

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

#endif /* EXODUSFORMATTER_H */
