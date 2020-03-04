//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest/gtest.h"
#include "PorousFlowConnectedNodes.h"

class PorousFlowConnectedNodesTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    _n1 = PorousFlowConnectedNodes();
    _n1.addGlobalNode(1);
    _n1.addGlobalNode(12);
    _n1.addGlobalNode(123);
    _n1.addGlobalNode(1234);
    _n1.finalizeAddingGlobalNodes();
    _n1.addConnection(1, 1);
    _n1.addConnection(1, 12);
    _n1.addConnection(1, 123);
    _n1.addConnection(1, 123);
    _n1.addConnection(1, 1234);
    _n1.addConnection(123, 1234);
    _n1.addConnection(12, 1234);
    _n1.finalizeAddingConnections();

    _n2 = PorousFlowConnectedNodes();
    _n2.addGlobalNode(2);
    _n2.addGlobalNode(4);
    _n2.addGlobalNode(6);
  }

  PorousFlowConnectedNodes _n1;
  PorousFlowConnectedNodes _n2;
};
