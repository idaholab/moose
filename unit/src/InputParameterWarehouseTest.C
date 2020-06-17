//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "InputParameterWarehouse.h"
#include "InputParameters.h"

TEST(InputParameterWarehouse, getControllableItems)
{
  // One item
  {
    InputParameters in_params = emptyInputParameters();
    in_params.addPrivateParam<std::string>("_moose_base", "Base");
    in_params.addParam<int>("control", 1949, "");
    in_params.declareControllable("control");

    InputParameterWarehouse wh;
    const InputParameters & params = wh.addInputParameters("Object", in_params);

    MooseObjectParameterName name("Base", "Object", "control");
    auto items = wh.getControllableItems(name);
    ASSERT_EQ(items.size(), 1);
    auto item = items[0];

    EXPECT_EQ(item->get<int>(), std::vector<int>(1, 1949));
    EXPECT_EQ(params.get<int>("control"), 1949);
    item->set<int>(1980);
    EXPECT_EQ(item->get<int>(), std::vector<int>(1, 1980));
    EXPECT_EQ(params.get<int>("control"), 1980);
    EXPECT_EQ(item->name(), name);
    EXPECT_EQ(item->type(), "int");
  }

  // Many items
  {
    InputParameters in_params = emptyInputParameters();
    in_params.addPrivateParam<std::string>("_moose_base", "Base");
    in_params.addParam<int>("control", 1949, "");
    in_params.addParam<int>("control2", 1954, "");

    in_params.declareControllable("control control2");

    InputParameterWarehouse wh;
    const InputParameters & params = wh.addInputParameters("Object", in_params);

    MooseObjectParameterName name("Base", "Object", "*");
    auto items = wh.getControllableItems(name);
    ASSERT_EQ(items.size(), 2);

    EXPECT_EQ(params.get<int>("control"), 1949);
    EXPECT_EQ(items[0]->get<int>(), std::vector<int>(1, 1949));
    EXPECT_EQ(params.get<int>("control2"), 1954);
    EXPECT_EQ(items[1]->get<int>(), std::vector<int>(1, 1954));
    items[0]->set<int>(1980);
    EXPECT_EQ(params.get<int>("control"), 1980);
    EXPECT_EQ(items[0]->get<int>(), std::vector<int>(1, 1980));
    EXPECT_EQ(params.get<int>("control2"), 1954);
    EXPECT_EQ(items[1]->get<int>(), std::vector<int>(1, 1954));
    items[1]->set<int>(1981);
    EXPECT_EQ(params.get<int>("control"), 1980);
    EXPECT_EQ(items[0]->get<int>(), std::vector<int>(1, 1980));
    EXPECT_EQ(params.get<int>("control2"), 1981);
    EXPECT_EQ(items[1]->get<int>(), std::vector<int>(1, 1981));
  }
}

TEST(InputParameterWarehouse, getControllableParameter)
{
  InputParameters in_params = emptyInputParameters();
  in_params.addPrivateParam<std::string>("_moose_base", "Base");
  in_params.addParam<int>("control", 2011, "");
  in_params.addParam<int>("control2", 2013, "");
  in_params.addParam<int>("no_control", 2009, "");
  in_params.declareControllable("control control2");

  InputParameterWarehouse wh;
  const InputParameters & params = wh.addInputParameters("Object", in_params);

  MooseObjectParameterName name("Base", "Object", "*");
  auto cp = wh.getControllableParameter(name);

  ASSERT_FALSE(cp.empty());
  EXPECT_EQ(cp.get<int>()[0], 2011);
  EXPECT_EQ(params.get<int>("control"), 2011);
  EXPECT_EQ(cp.get<int>()[1], 2013);
  EXPECT_EQ(params.get<int>("control2"), 2013);
  cp.set<int>(2009);
  EXPECT_EQ(cp.get<int>()[0], 2009);
  EXPECT_EQ(params.get<int>("control"), 2009);
  EXPECT_EQ(cp.get<int>()[1], 2009);
  EXPECT_EQ(params.get<int>("control2"), 2009);
}

TEST(InputParameterWarehouse, getControllableParameterValues)
{
  InputParameters in_params = emptyInputParameters();
  in_params.addPrivateParam<std::string>("_moose_base", "Base");
  in_params.addParam<int>("control", 2011, "");
  in_params.declareControllable("control");

  InputParameterWarehouse wh;
  wh.addInputParameters("Object", in_params);

  MooseObjectParameterName name("Base", "Object", "*");
  std::vector<int> values = wh.getControllableParameterValues<int>(name);

  ASSERT_FALSE(values.empty());
  EXPECT_EQ(values, std::vector<int>(1, 2011));
}

TEST(InputParameterWarehouse, emptyControllableParameterValues)
{
  InputParameters in_params = emptyInputParameters();
  in_params.addPrivateParam<std::string>("_moose_base", "Base");
  in_params.addParam<int>("control", 2011, "");
  in_params.declareControllable("control");

  InputParameterWarehouse wh;
  wh.addInputParameters("Object", in_params);

  MooseObjectParameterName name("Base", "Object", "asdf");
  std::vector<int> values = wh.getControllableParameterValues<int>(name);

  ASSERT_TRUE(values.empty());
}

TEST(InputParameterWarehouse, addControllableParameterConnection)
{
  // One-to-one
  {
    InputParameters in_params = emptyInputParameters();
    in_params.addPrivateParam<std::string>("_moose_base", "Base");
    in_params.addParam<int>("control", 1949, "");
    in_params.declareControllable("control");

    InputParameterWarehouse wh;
    const InputParameters & params0 = wh.addInputParameters("Object0", in_params);
    in_params.set<int>("control") = 1954;
    const InputParameters & params1 = wh.addInputParameters("Object1", in_params);

    MooseObjectParameterName name0("Base", "Object0", "control");
    MooseObjectParameterName name1("Base", "Object1", "control");
    wh.addControllableParameterConnection(name0, name1);

    auto cp = wh.getControllableParameter(name0);
    ASSERT_EQ(cp.get<int>().size(), 2);
    EXPECT_EQ(params0.get<int>("control"), 1949);
    EXPECT_EQ(cp.get<int>()[0], 1949);
    EXPECT_EQ(params1.get<int>("control"), 1954);
    EXPECT_EQ(cp.get<int>()[1], 1954);
    cp.set<int>(1980);
    ASSERT_EQ(cp.get<int>().size(), 2);
    EXPECT_EQ(cp.get<int>()[0], 1980);
    EXPECT_EQ(params0.get<int>("control"), 1980);
    EXPECT_EQ(cp.get<int>()[1], 1980);
    EXPECT_EQ(params1.get<int>("control"), 1980);
  }

  // One-to-many
  {
    InputParameters in_params = emptyInputParameters();
    in_params.addPrivateParam<std::string>("_moose_base", "Base");
    in_params.addParam<int>("control", 1949, "");
    in_params.declareControllable("control");

    InputParameterWarehouse wh;
    const InputParameters & params0 = wh.addInputParameters("Object0", in_params);

    in_params.set<int>("control") = 2011;
    in_params.set<std::string>("_moose_base") = "Base2";
    const InputParameters & params1 = wh.addInputParameters("Object1", in_params);
    in_params.set<int>("control") = 2013;
    const InputParameters & params2 = wh.addInputParameters("Object2", in_params);

    MooseObjectParameterName name0("Base", "Object0", "control");
    MooseObjectParameterName name1("Base2", "*", "control");
    wh.addControllableParameterConnection(name0, name1);

    auto cp = wh.getControllableParameter(name0);
    ASSERT_EQ(cp.get<int>().size(), 3);
    EXPECT_EQ(params0.get<int>("control"), 1949);
    EXPECT_EQ(cp.get<int>()[0], 1949);
    EXPECT_EQ(params1.get<int>("control"), 2011);
    EXPECT_EQ(cp.get<int>()[1], 2011);
    EXPECT_EQ(params2.get<int>("control"), 2013);
    EXPECT_EQ(cp.get<int>()[2], 2013);

    cp.set<int>(1980);
    ASSERT_EQ(cp.get<int>().size(), 3);
    EXPECT_EQ(cp.get<int>()[0], 1980);
    EXPECT_EQ(params0.get<int>("control"), 1980);
    EXPECT_EQ(cp.get<int>()[1], 1980);
    EXPECT_EQ(params1.get<int>("control"), 1980);
    EXPECT_EQ(cp.get<int>()[2], 1980);
    EXPECT_EQ(params2.get<int>("control"), 1980);
  }

  // Many-to-one
  {
    InputParameters in_params = emptyInputParameters();
    in_params.addPrivateParam<std::string>("_moose_base", "Base");
    in_params.addParam<int>("control", 1949, "");
    in_params.declareControllable("control");

    InputParameterWarehouse wh;
    const InputParameters & params0 = wh.addInputParameters("Object0", in_params);

    in_params.set<int>("control") = 2011;
    in_params.set<std::string>("_moose_base") = "Base2";
    const InputParameters & params1 = wh.addInputParameters("Object1", in_params);
    in_params.set<int>("control") = 2013;
    const InputParameters & params2 = wh.addInputParameters("Object2", in_params);

    MooseObjectParameterName name0("Base", "Object0", "control");
    MooseObjectParameterName name1("Base2", "Object1", "control");
    MooseObjectParameterName name2("Base2", "Object2", "control");
    MooseObjectParameterName name("Base2", "*", "control");
    wh.addControllableParameterConnection(name, name0);

    auto cp1 = wh.getControllableParameter(name1);
    auto cp2 = wh.getControllableParameter(name2);

    EXPECT_EQ(params0.get<int>("control"), 1949);
    EXPECT_EQ(params1.get<int>("control"), 2011);
    EXPECT_EQ(params2.get<int>("control"), 2013);

    ASSERT_EQ(cp1.get<int>().size(), 2);
    EXPECT_EQ(cp1.get<int>()[0], 2011);
    EXPECT_EQ(cp1.get<int>()[1], 1949);

    ASSERT_EQ(cp2.get<int>().size(), 2);
    EXPECT_EQ(cp2.get<int>()[0], 2013);
    EXPECT_EQ(cp2.get<int>()[1], 1949);

    cp1.set<int>(1980);

    EXPECT_EQ(params0.get<int>("control"), 1980);
    EXPECT_EQ(params1.get<int>("control"), 1980);
    EXPECT_EQ(params2.get<int>("control"), 2013);

    ASSERT_EQ(cp1.get<int>().size(), 2);
    EXPECT_EQ(cp1.get<int>()[0], 1980);
    EXPECT_EQ(cp1.get<int>()[1], 1980);

    ASSERT_EQ(cp2.get<int>().size(), 2);
    EXPECT_EQ(cp2.get<int>()[0], 2013);
    EXPECT_EQ(cp2.get<int>()[1], 1980);

    cp2.set<int>(1906);

    EXPECT_EQ(params0.get<int>("control"), 1906);
    EXPECT_EQ(params1.get<int>("control"), 1980);
    EXPECT_EQ(params2.get<int>("control"), 1906);

    ASSERT_EQ(cp1.get<int>().size(), 2);
    EXPECT_EQ(cp1.get<int>()[0], 1980);
    EXPECT_EQ(cp1.get<int>()[1], 1906);

    ASSERT_EQ(cp2.get<int>().size(), 2);
    EXPECT_EQ(cp2.get<int>()[0], 1906);
    EXPECT_EQ(cp2.get<int>()[1], 1906);
  }

  // Many-to-many
  {
    InputParameters in_params = emptyInputParameters();
    in_params.addPrivateParam<std::string>("_moose_base", "Base");
    in_params.addParam<int>("control", 1949, "");
    in_params.declareControllable("control");

    InputParameterWarehouse wh;
    const InputParameters & params0 = wh.addInputParameters("Object0", in_params);
    in_params.addParam<int>("control", 1954, "");
    const InputParameters & params1 = wh.addInputParameters("Object1", in_params);

    in_params.set<int>("control") = 2011;
    in_params.set<std::string>("_moose_base") = "Base2";
    const InputParameters & params2 = wh.addInputParameters("Object2", in_params);
    in_params.set<int>("control") = 2013;
    const InputParameters & params3 = wh.addInputParameters("Object3", in_params);

    MooseObjectParameterName name0("Base", "Object0", "control");
    MooseObjectParameterName name1("Base", "Object1", "control");
    MooseObjectParameterName name2("Base2", "Object2", "control");
    MooseObjectParameterName name3("Base2", "Object3", "control");

    MooseObjectParameterName name_a("Base", "*", "control");
    MooseObjectParameterName name_b("Base2", "*", "control");
    wh.addControllableParameterConnection(name_a, name_b);

    auto cp0 = wh.getControllableParameter(name0);
    auto cp1 = wh.getControllableParameter(name1);
    auto cp2 = wh.getControllableParameter(name2);
    auto cp3 = wh.getControllableParameter(name3);

    EXPECT_EQ(params0.get<int>("control"), 1949);
    EXPECT_EQ(params1.get<int>("control"), 1954);
    EXPECT_EQ(params2.get<int>("control"), 2011);
    EXPECT_EQ(params3.get<int>("control"), 2013);

    std::vector<int> gold;
    ASSERT_EQ(cp0.get<int>().size(), 3);
    ASSERT_EQ(cp1.get<int>().size(), 3);
    ASSERT_EQ(cp2.get<int>().size(), 1);
    ASSERT_EQ(cp3.get<int>().size(), 1);

    gold = {1949, 2011, 2013};
    EXPECT_EQ(cp0.get<int>(), gold);
    gold = {1954, 2011, 2013};
    EXPECT_EQ(cp1.get<int>(), gold);
    EXPECT_EQ(cp2.get<int>()[0], 2011);
    EXPECT_EQ(cp3.get<int>()[0], 2013);

    cp0.set<int>(1980);

    EXPECT_EQ(params0.get<int>("control"), 1980);
    EXPECT_EQ(params1.get<int>("control"), 1954);
    EXPECT_EQ(params2.get<int>("control"), 1980);
    EXPECT_EQ(params3.get<int>("control"), 1980);

    ASSERT_EQ(cp0.get<int>().size(), 3);
    ASSERT_EQ(cp1.get<int>().size(), 3);
    ASSERT_EQ(cp2.get<int>().size(), 1);
    ASSERT_EQ(cp3.get<int>().size(), 1);

    gold = {1980, 1980, 1980};
    EXPECT_EQ(cp0.get<int>(), gold);
    gold = {1954, 1980, 1980};
    EXPECT_EQ(cp1.get<int>(), gold);
    EXPECT_EQ(cp2.get<int>()[0], 1980);
    EXPECT_EQ(cp3.get<int>()[0], 1980);

    cp1.set<int>(1906);

    EXPECT_EQ(params0.get<int>("control"), 1980);
    EXPECT_EQ(params1.get<int>("control"), 1906);
    EXPECT_EQ(params2.get<int>("control"), 1906);
    EXPECT_EQ(params3.get<int>("control"), 1906);

    ASSERT_EQ(cp0.get<int>().size(), 3);
    ASSERT_EQ(cp1.get<int>().size(), 3);
    ASSERT_EQ(cp2.get<int>().size(), 1);
    ASSERT_EQ(cp3.get<int>().size(), 1);

    gold = {1980, 1906, 1906};
    EXPECT_EQ(cp0.get<int>(), gold);
    gold = {1906, 1906, 1906};
    EXPECT_EQ(cp1.get<int>(), gold);
    EXPECT_EQ(cp2.get<int>()[0], 1906);
    EXPECT_EQ(cp3.get<int>()[0], 1906);

    cp2.set<int>(1907);

    EXPECT_EQ(params0.get<int>("control"), 1980);
    EXPECT_EQ(params1.get<int>("control"), 1906);
    EXPECT_EQ(params2.get<int>("control"), 1907);
    EXPECT_EQ(params3.get<int>("control"), 1906);

    ASSERT_EQ(cp0.get<int>().size(), 3);
    ASSERT_EQ(cp1.get<int>().size(), 3);
    ASSERT_EQ(cp2.get<int>().size(), 1);
    ASSERT_EQ(cp3.get<int>().size(), 1);

    gold = {1980, 1907, 1906};
    EXPECT_EQ(cp0.get<int>(), gold);
    gold = {1906, 1907, 1906};
    EXPECT_EQ(cp1.get<int>(), gold);
    EXPECT_EQ(cp2.get<int>()[0], 1907);
    EXPECT_EQ(cp3.get<int>()[0], 1906);

    cp3.set<int>(2009);

    EXPECT_EQ(params0.get<int>("control"), 1980);
    EXPECT_EQ(params1.get<int>("control"), 1906);
    EXPECT_EQ(params2.get<int>("control"), 1907);
    EXPECT_EQ(params3.get<int>("control"), 2009);

    ASSERT_EQ(cp0.get<int>().size(), 3);
    ASSERT_EQ(cp1.get<int>().size(), 3);
    ASSERT_EQ(cp2.get<int>().size(), 1);
    ASSERT_EQ(cp3.get<int>().size(), 1);

    gold = {1980, 1907, 2009};
    EXPECT_EQ(cp0.get<int>(), gold);
    gold = {1906, 1907, 2009};
    EXPECT_EQ(cp1.get<int>(), gold);
    EXPECT_EQ(cp2.get<int>()[0], 1907);
    EXPECT_EQ(cp3.get<int>()[0], 2009);
  }
}

TEST(InputParameterWarehouse, addControllableParameterAlias)
{
  InputParameters in_params = emptyInputParameters();
  in_params.addPrivateParam<std::string>("_moose_base", "Base");
  in_params.addParam<int>("control", 1949, "");
  in_params.declareControllable("control");

  InputParameterWarehouse wh;
  const InputParameters & params = wh.addInputParameters("Object", in_params);

  MooseObjectParameterName alias("not", "a", "param");
  MooseObjectParameterName secondary("Base", "Object", "control");
  wh.addControllableParameterAlias(alias, secondary);

  auto cp = wh.getControllableParameter(alias);

  ASSERT_FALSE(cp.empty());
  EXPECT_EQ(params.get<int>("control"), 1949);
  EXPECT_EQ(cp.get<int>()[0], 1949);
  cp.set<int>(1980);
  EXPECT_EQ(params.get<int>("control"), 1980);
  EXPECT_EQ(cp.get<int>()[0], 1980);
}
