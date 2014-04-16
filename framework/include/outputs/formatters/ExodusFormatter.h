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

#ifndef EXODUSFORMATTER_H
#define EXODUSFORMATTER_H

#include "InputFileFormatter.h"

class ExodusFormatter : public InputFileFormatter
{
public:
  ExodusFormatter();

  virtual void printInputFile(ActionWarehouse & wh);

  virtual void format();

  std::vector<std::string> & getInputFileRecord() { return _input_file_record; }

protected:
  std::stringstream _ss;
  std::vector<std::string> _input_file_record;
};

#endif /* EXODUSFORMATTER_H */
