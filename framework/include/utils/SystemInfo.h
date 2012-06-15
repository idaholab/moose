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

#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <string>
#include <ctime>

class SystemInfo
{
public:
  SystemInfo(int argc, char *argv[]);

  std::string getInfo();
  std::string getTimeStamp(time_t *time_stamp=NULL);

protected:
  int _argc;
  char **_argv;
};

#endif // SYSTEMINFO_H
