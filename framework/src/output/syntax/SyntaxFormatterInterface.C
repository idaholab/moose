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

#include "SyntaxFormatterInterface.h"

SyntaxFormatterInterface::SyntaxFormatterInterface(std::ostream & out, bool dump_mode)
  :_out(out),
   _dump_mode(dump_mode)
   
{
}

SyntaxFormatterInterface::~SyntaxFormatterInterface()
{
}
