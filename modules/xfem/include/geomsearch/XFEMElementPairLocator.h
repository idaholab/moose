//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPairLocator.h"
#include "XFEM.h"

class XFEMElementPairLocator : public ElementPairLocator
{
public:
  XFEMElementPairLocator(std::shared_ptr<XFEM> xfem,
                         unsigned int interface_id,
                         bool use_displaced_mesh = false);
  virtual void reinit();
  virtual void update();

protected:
  std::shared_ptr<XFEM> _xfem;
  bool _use_displaced_mesh;
};
