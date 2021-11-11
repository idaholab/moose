//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

///@{ Create global and threaded shared memory for use in external plugins
extern "C" int * SMAIntArrayCreate(int id, int len, int val = 0);
extern "C" Real * SMAFloatArrayCreate(int id, int len, Real val = 0.0);
extern "C" int * SMALocalIntArrayCreate(int id, int len, int val = 0);
extern "C" Real * SMALocalFloatArrayCreate(int id, int len, Real val = 0.0);
///@}

///@{ Access global and threaded shared memory for use in external plugins
extern "C" int * SMAIntArrayAccess(int id);
extern "C" Real * SMAFloatArrayAccess(int id);
extern "C" int * SMALocalIntArrayAccess(int id);
extern "C" Real * SMALocalFloatArrayAccess(int id);
///@}

///@{ Query size of global and threaded shared memory for use in external plugins
extern "C" std::size_t SMAIntArraySize(int id);
extern "C" std::size_t SMAFloatArraySize(int id);
extern "C" std::size_t SMALocalIntArraySize(int id);
extern "C" std::size_t SMALocalFloatArraySize(int id);
///@}

///@{ Delete global and threaded shared memory for use in external plugins
extern "C" void SMAIntArrayDelete(int id);
extern "C" void SMAFloatArrayDelete(int id);
extern "C" void SMALocalIntArrayDelete(int id);
extern "C" void SMALocalFloatArrayDelete(int id);
///@}

/// Get ID of the current thread
extern "C" int get_thread_id();

/// Get total number of threads for the current process
extern "C" int getnumthreads();

///@{ Mutex utility functions for use in external plugins with threaded execution
extern "C" void MutexInit(int id);
extern "C" void MutexLock(int id);
extern "C" void MutexUnlock(int id);
///@}
