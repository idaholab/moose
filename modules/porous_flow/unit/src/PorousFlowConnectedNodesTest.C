//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowConnectedNodesTest.h"

TEST_F(PorousFlowConnectedNodesTest, errors)
{
  try
  {
    _n1.addGlobalNode(1);
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find("PorousFlowConnectedNodes: addGlobalNode called, but _still_adding_global_nodes "
                  "is false.  You possibly called finalizeAddingGlobalNodes too soon.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.globalID(1);
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find("PorousFlowConnectedNodes: globalID called, but _still_adding_global_nodes is "
                  "true.  Probably you should have called finalizeAddingGlobalNodes.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.globalIDs();
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find("PorousFlowConnectedNodes: globalIDs called, but _still_adding_global_nodes is "
                  "true.  Probably you should have called finalizeAddingGlobalNodes.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.sequentialID(1);
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find("PorousFlowConnectedNodes: sequentialID called, but _still_adding_global_nodes "
                  "is true.  Probably you should have called finalizeAddingGlobalNodes.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.addConnection(1, 1);
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find("PorousFlowConnectedNodes: addConnection called, but _still_adding_global_nodes "
                  "is true.  Probably you should have called finalizeAddingGlobalNodes.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n1.addConnection(1, 1);
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find("PorousFlowConnectedNodes: addConnection called, but _still_adding_connections "
                  "is false.  Probably you should have called finalizeAddingConnections.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.sequentialConnectionsToGlobalID(1);
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("PorousFlowConnectedNodes: sequentialConnectionsToGlobalID called, "
                                "but _still_adding_connections is true.  Probably you should have "
                                "called finalizeAddingConnections.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.sequentialConnectionsToSequentialID(1);
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("PorousFlowConnectedNodes: sequentialConnectionsToSequentialID "
                                "called, but _still_adding_connections is true.  Probably you "
                                "should have called finalizeAddingConnections.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.globalConnectionsToGlobalID(1);
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("PorousFlowConnectedNodes: globalConnectionsToGlobalID called, but "
                                "_still_adding_connections is true.  Probably you should have "
                                "called finalizeAddingConnections.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.globalConnectionsToSequentialID(1);
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("PorousFlowConnectedNodes: globalConnectionsToSequentialID called, "
                                "but _still_adding_connections is true.  Probably you should have "
                                "called finalizeAddingConnections.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.indexOfSequentialConnection(0, 0);
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("PorousFlowConnectedNodes: indexOfSequentialConnection called, but "
                                "_still_adding_connections is true.  Probably you should have "
                                "called finalizeAddingConnections.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n1.indexOfSequentialConnection(1, 0);
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("PorousFlowConnectedNode: sequential_node_ID_from 1 has no "
                                "connection to sequential_node_ID_to 0");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.indexOfGlobalConnection(0, 0);
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("PorousFlowConnectedNodes: indexOfGlobalConnection called, but "
                                "_still_adding_connections is true.  Probably you should have "
                                "called finalizeAddingConnections.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n1.indexOfGlobalConnection(1, 12);
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("PorousFlowConnectedNode: global_ID_from 1 has no connection to "
                                "global_node_ID_to 12");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _n2.sizeSequential();
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find("PorousFlowConnectedNodes: sizeSequential called, but _still_adding_global_nodes "
                  "is true.  Probably you should have called finalizeAddingGlobalNodes.");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST_F(PorousFlowConnectedNodesTest, numNodes)
{
  EXPECT_EQ(_n1.numNodes(), (std::size_t)4);
  EXPECT_EQ(_n2.numNodes(), (std::size_t)3);
}

TEST_F(PorousFlowConnectedNodesTest, globalID)
{
  EXPECT_EQ(_n1.globalID(0), (dof_id_type)1);
  EXPECT_EQ(_n1.globalID(1), (dof_id_type)12);
  EXPECT_EQ(_n1.globalID(2), (dof_id_type)123);
  EXPECT_EQ(_n1.globalID(3), (dof_id_type)1234);
  EXPECT_EQ(_n1.globalIDs()[0], (dof_id_type)1);
  EXPECT_EQ(_n1.globalIDs()[1], (dof_id_type)12);
  EXPECT_EQ(_n1.globalIDs()[2], (dof_id_type)123);
  EXPECT_EQ(_n1.globalIDs()[3], (dof_id_type)1234);
}

TEST_F(PorousFlowConnectedNodesTest, sequentialID)
{
  EXPECT_EQ(_n1.sequentialID(1), (dof_id_type)0);
  EXPECT_EQ(_n1.sequentialID(12), (dof_id_type)1);
  EXPECT_EQ(_n1.sequentialID(123), (dof_id_type)2);
  EXPECT_EQ(_n1.sequentialID(1234), (dof_id_type)3);
}

TEST_F(PorousFlowConnectedNodesTest, connections)
{
  EXPECT_EQ(_n1.sequentialConnectionsToGlobalID(1)[0], (dof_id_type)0);
  EXPECT_EQ(_n1.sequentialConnectionsToGlobalID(1)[1], (dof_id_type)1);
  EXPECT_EQ(_n1.sequentialConnectionsToGlobalID(1)[2], (dof_id_type)2);
  EXPECT_EQ(_n1.sequentialConnectionsToGlobalID(1)[3], (dof_id_type)3);
  EXPECT_EQ(_n1.sequentialConnectionsToGlobalID(12)[0], (dof_id_type)3);
  EXPECT_EQ(_n1.sequentialConnectionsToGlobalID(123)[0], (dof_id_type)3);

  EXPECT_EQ(_n1.sequentialConnectionsToSequentialID(0)[0], (dof_id_type)0);
  EXPECT_EQ(_n1.sequentialConnectionsToSequentialID(0)[1], (dof_id_type)1);
  EXPECT_EQ(_n1.sequentialConnectionsToSequentialID(0)[2], (dof_id_type)2);
  EXPECT_EQ(_n1.sequentialConnectionsToSequentialID(0)[3], (dof_id_type)3);
  EXPECT_EQ(_n1.sequentialConnectionsToSequentialID(1)[0], (dof_id_type)3);
  EXPECT_EQ(_n1.sequentialConnectionsToSequentialID(2)[0], (dof_id_type)3);

  EXPECT_EQ(_n1.indexOfSequentialConnection(0, 0), (unsigned)0);
  EXPECT_EQ(_n1.indexOfSequentialConnection(0, 1), (unsigned)1);
  EXPECT_EQ(_n1.indexOfSequentialConnection(0, 2), (unsigned)2);
  EXPECT_EQ(_n1.indexOfSequentialConnection(0, 3), (unsigned)3);
  EXPECT_EQ(_n1.indexOfSequentialConnection(1, 3), (unsigned)0);
  EXPECT_EQ(_n1.indexOfSequentialConnection(2, 3), (unsigned)0);

  EXPECT_EQ(_n1.globalConnectionsToGlobalID(1)[0], (dof_id_type)1);
  EXPECT_EQ(_n1.globalConnectionsToGlobalID(1)[1], (dof_id_type)12);
  EXPECT_EQ(_n1.globalConnectionsToGlobalID(1)[2], (dof_id_type)123);
  EXPECT_EQ(_n1.globalConnectionsToGlobalID(1)[3], (dof_id_type)1234);
  EXPECT_EQ(_n1.globalConnectionsToGlobalID(12)[0], (dof_id_type)1234);
  EXPECT_EQ(_n1.globalConnectionsToGlobalID(123)[0], (dof_id_type)1234);

  EXPECT_EQ(_n1.indexOfGlobalConnection(1, 1), (unsigned)0);
  EXPECT_EQ(_n1.indexOfGlobalConnection(1, 12), (unsigned)1);
  EXPECT_EQ(_n1.indexOfGlobalConnection(1, 123), (unsigned)2);
  EXPECT_EQ(_n1.indexOfGlobalConnection(1, 1234), (unsigned)3);
  EXPECT_EQ(_n1.indexOfGlobalConnection(12, 1234), (unsigned)0);
  EXPECT_EQ(_n1.indexOfGlobalConnection(123, 1234), (unsigned)0);

  EXPECT_EQ(_n1.globalConnectionsToSequentialID(0)[0], (dof_id_type)1);
  EXPECT_EQ(_n1.globalConnectionsToSequentialID(0)[1], (dof_id_type)12);
  EXPECT_EQ(_n1.globalConnectionsToSequentialID(0)[2], (dof_id_type)123);
  EXPECT_EQ(_n1.globalConnectionsToSequentialID(0)[3], (dof_id_type)1234);
  EXPECT_EQ(_n1.globalConnectionsToSequentialID(1)[0], (dof_id_type)1234);
  EXPECT_EQ(_n1.globalConnectionsToSequentialID(2)[0], (dof_id_type)1234);
}

TEST_F(PorousFlowConnectedNodesTest, sizeSequential)
{
  EXPECT_EQ(_n1.sizeSequential(), (std::size_t)1234);
}
