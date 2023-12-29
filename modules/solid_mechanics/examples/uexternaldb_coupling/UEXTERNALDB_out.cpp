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
std::ofstream out_file;

extern "C" void FOR_NAME(uexternaldb, UEXTERNALDB)(const int & LOP,
                                                   const int &,
                                                   const double (&)[2],
                                                   const double &,
                                                   const int &,
                                                   const int & step)
{
  int myNthreads = CALL_NAME(getnumthreads, GETNUMTHREADS)();
  int myThreadID = CALL_NAME(get_thread_id, GET_THREAD_ID)();

  int rank;
  CALL_NAME(getrank, GETRANK)(&rank);
  int nrank;
  CALL_NAME(getnumcpus, GETNUMCPUS)(&nrank);

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

    if (rank == 0)
    {
      out_file.open("UEXTERNALDB_out.csv");
      out_file << "step,element,qp,increment\n";
    }

    std::cout << "UEXTERNALDB: Done with LOP = 0" << std::endl;
  }
  else if (LOP == 2)
  {
    if (myThreadID > 0)
      return;

    auto comm = get_communicator();

    std::cout << "UEXTERNALDB: Starting LOP = 2" << std::endl;

    // combine data from all threads on the current MPI rank
    std::vector<int> int_buf1, int_buf2;
    std::vector<double> double_buf;
    for (int i = 0; i < myNthreads; ++i)
      for (auto & j : out_data[i])
      {
        int_buf1.push_back(j.first.first);
        int_buf2.push_back(j.first.second);
        double_buf.push_back(j.second);
      }

    // how many value triplets does each rank hold?
    std::vector<int> counts(nrank);
    int nelements = int_buf1.size();
    MPI_Gather(&nelements, 1, MPI_INT, counts.data(), 1, MPI_INT, 0, comm);

    // Displacements in the receive buffer for MPI_GATHERV
    std::vector<int> disps(nrank);
    for (int i = 0; i < nrank; ++i)
      disps[i] = (i > 0) ? (disps[i - 1] + counts[i - 1]) : 0;

    // Allocate receive buffer only on root
    int size = disps[nrank - 1] + counts[nrank - 1];
    std::vector<int> int_rcv1, int_rcv2;
    std::vector<double> double_rcv;
    if (rank == 0)
    {
      int_rcv1.resize(size);
      int_rcv2.resize(size);
      double_rcv.resize(size);
    }

    // parallel communication
    MPI_Gatherv(int_buf1.data(),
                nelements,
                MPI_INT,
                int_rcv1.data(),
                counts.data(),
                disps.data(),
                MPI_INT,
                0,
                comm);
    MPI_Gatherv(int_buf2.data(),
                nelements,
                MPI_INT,
                int_rcv2.data(),
                counts.data(),
                disps.data(),
                MPI_INT,
                0,
                comm);
    MPI_Gatherv(double_buf.data(),
                nelements,
                MPI_DOUBLE,
                double_rcv.data(),
                counts.data(),
                disps.data(),
                MPI_DOUBLE,
                0,
                comm);

    // put data back into map for sorted output
    for (unsigned int i = 0; i < int_rcv1.size(); ++i)
      out_data[0][std::make_pair(int_rcv1[i], int_rcv2[i])] = double_rcv[i];

    // write file at root only
    if (rank == 0)
      for (auto item : out_data[0])
        out_file << step << ',' << item.first.first << ',' << item.first.second << ','
                 << item.second << '\n';

    std::cout << "UEXTERNALDB: Done with LOP = 2" << std::endl;
  }
  else if (LOP == 3)
  {
    if (rank == 0)
      out_file.close();
  }
}
