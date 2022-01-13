//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SMAAspUserSubroutines.h"
#include "SMAAspUserUtilities.h"
#include "omi_for_c.h"

#include <unistd.h>
#include <limits.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// data shared with the UMAT object
std::vector<std::map<std::pair<int, int>, double>> out_data;

extern "C" void FOR_NAME(uexternaldb, UEXTERNALDB)(
    const int & LOP, const int &, const double (&)[2], const double &, const int &, const int &)
{
  int myNthreads = CALL_NAME(getnumthreads, GETNUMTHREADS)();
  int myThreadID = CALL_NAME(get_thread_id, GET_THREAD_ID)();

  if (LOP == 0)
  {
    std::cout << "UEXTERNALDB: Starting LOP = 0" << std::endl;
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
      std::cout << "UEXTERNALDB: &&&&&&&&&&&&" << std::endl;
      std::cout << "UEXTERNALDB: Current working directory: " << cwd << std::endl;
      std::cout << "UEXTERNALDB: &&&&&&&&&&&&" << std::endl;
    }
    char outdir[256];

    int lenout;
    // int GETLEN(outdir) = 256;
    CALL_NAME(getoutdir, GETOUTDIR)(CHNAME_C(outdir), &lenout);
    std::cout << "outdir name is ";
    for (int i = 0; i < lenout; ++i)
      std::cout << outdir[i];
    std::cout << std::endl;

    char jobname[256];
    int jobnamelen;
    // int GETLEN(jobname) = 256;
    CALL_NAME(getjobname, GETJOBNAME)(CHNAME_C(jobname), &jobnamelen);
    int namecount(0);
    std::string jobnamestr;
    while (jobname[namecount] != ' ') // NULL
      namecount++;
    for (int i = 0; i < namecount; ++i)
      jobnamestr += jobname[i];

    std::cout << "size of file name is " << jobnamestr.length() << std::endl;
    std::cout << "job name is " << jobnamestr << std::endl;

    std::cout << "UEXTERNALDB: The number of threads is " << myNthreads << std::endl;

    // every thread creates a map of number of threads size
    if (myThreadID == 0)
      out_data.resize(myNthreads);

    std::cout << "UEXTERNALDB: Done with LOP = 0" << std::endl;
  }
  else if (LOP == 2)
  {
    std::cout << "UEXTERNALDB: Starting LOP = 2" << std::endl;
    if (myThreadID == 0)
    {
      std::ofstream out;
      out.open("UEXTERNALDB_out.csv");
      out << "element,qp,increment\n";
      for (int i = 0; i < myNthreads; ++i)
        for (auto & j : out_data[i])
          out << j.first.first << ',' << j.first.second << ',' << j.second << '\n';
      out.close();
    }
    std::cout << "UEXTERNALDB: Done with LOP = 2" << std::endl;
  }
}
