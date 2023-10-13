//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVThread.h"

LinearFVThread::LinearFVThread(FEProblemBase & fe_problem) : LinearThread(fe_problem) {}

// Splitting Constructor
LinearFVThread::LinearFVThread(LinearFVThread & x, Threads::split split) : LinearThread(x, split) {}

LinearFVThread::~LinearFVThread() {}

void
LinearFVThread::computeOnBoundary(BoundaryID /*bnd_id*/, const Elem * /*lower_d_elem*/)
{
}

void
LinearFVThread::computeOnElement()
{
}

void LinearFVThread::computeOnInterface(BoundaryID /*bnd_id*/) {}

void
LinearFVThread::computeOnInternalFace(const Elem * /*neighbor*/)
{
}
