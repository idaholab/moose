//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubChannelTestApp.h"
#include "MooseMain.h"

int
main(int argc, char * argv[])
{
  Moose::main<SubChannelTestApp>(argc, argv);

  return 0;
}
