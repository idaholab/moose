//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

extern "C" int * SMAIntArrayCreate(int id, int len, int val = 0);
extern "C" double * SMAFloatArrayCreate(int id, int len, double val = 0.0);
extern "C" int * SMALocalIntArrayCreate(int id, int len, int val = 0);
extern "C" double * SMALocalFloatArrayCreate(int id, int len, double val = 0.0);

extern "C" int * SMAIntArrayAccess(int id);
extern "C" double * SMAFloatArrayAccess(int id);
extern "C" int * SMALocalIntArrayAccess(int id);
extern "C" double * SMALocalFloatArrayAccess(int id);

extern "C" int SMAIntArraySize(int id);
extern "C" int SMAFloatArraySize(int id);
extern "C" int SMALocalIntArraySize(int id);
extern "C" int SMALocalFloatArraySize(int id);

extern "C" void SMAIntArrayDelete(int id);
extern "C" void SMAFloatArrayDelete(int id);
extern "C" void SMALocalIntArrayDelete(int id);
extern "C" void SMALocalFloatArrayDelete(int id);

extern "C" void MutexInit(int id);
extern "C" void MutexLock(int id);
extern "C" void MutexUnlock(int id);
