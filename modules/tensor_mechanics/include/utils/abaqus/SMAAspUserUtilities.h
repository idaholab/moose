//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "mpi.h"

extern "C" int getnumthreads_();
extern "C" int get_thread_id_();

extern "C" void getoutdir_(char * dir, int * len);
extern "C" void getjobname_(char * dir, int * len);

extern "C" void getnumcpus_(int * num);
extern "C" void getrank_(int * rank);
extern "C" MPI_Comm get_communicator();

extern "C" void
stdb_abqerr_(int * lop, char * format, int * intv, double * realv, char * charv, int format_len);
