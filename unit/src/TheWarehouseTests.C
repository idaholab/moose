//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObjectUnitTest.h"
#include "TheWarehouse.h"
#include "MooseHashing.h"

#include "gtest_include.h"
#include <iostream>
#include <vector>
#include <unordered_map>

class TestObject : public MooseObject
{
public:
  TestObject(const InputParameters & params) : MooseObject(params) {}
  static InputParameters validParams();
  virtual bool enabled() const override { return on; }
  std::vector<int> vals;
  bool on = true;
};

registerMooseObject("MooseUnitApp", TestObject);

InputParameters
TestObject::validParams()
{
  auto p = MooseObject::validParams();
  p.registerBase("TestObject");
  return p;
}

const std::string &
fastStr(int i)
{
  static std::vector<std::string> vec;
  if (vec.empty())
    for (int i = 0; i < 20; i++)
      vec.push_back(std::to_string(i));
  return vec[i];
}

class TestAttrib : public Attribute
{
public:
  TestAttrib(TheWarehouse & w, int index, int val)
    : Attribute(w, fastStr(index)), index(index), val(val)
  {
  }

  virtual std::unique_ptr<Attribute> clone() const override
  {
    return std::unique_ptr<Attribute>(new TestAttrib(*this));
  }
  virtual void initFrom(const MooseObject * obj) override
  {
    auto o = static_cast<const TestObject *>(obj);
    val = o->vals[index];
  }

  virtual bool isMatch(const Attribute & other) const override
  {
    auto o = static_cast<const TestAttrib *>(&other);
    return val == o->val;
  }
  virtual bool isEqual(const Attribute & other) const override { return isMatch(other); }

  virtual size_t hash() const override
  {
    size_t h = 0;
    Moose::hash_combine(h, index, val);
    return h;
  }

  int index;
  int val;
};

class TheWarehouseTest : public MooseObjectUnitTest
{
public:
  TheWarehouseTest() : MooseObjectUnitTest("MooseUnitApp")
  {
    w.registerAttribute<TestAttrib>("0", 0, 0);
    w.registerAttribute<TestAttrib>("1", 1, 0);
    w.registerAttribute<TestAttrib>("2", 2, 0);
    w.registerAttribute<TestAttrib>("3", 3, 0);
    w.registerAttribute<AttribSorted>("sorted");
  }

  std::shared_ptr<TestObject> obj(int val1, int val2, int val3, int val4)
  {
    InputParameters p = _factory.getValidParams("TestObject");
    p.set<SubProblem *>("_subproblem") = _fe_problem.get();
    auto obj = _factory.create<TestObject>("TestObject", "dummy" + std::to_string(count++), p, 0);
    obj->vals = {val1, val2, val3, val4};
    return obj;
  }

  int count = 0;
  TheWarehouse w;
};

struct AttribSpec
{
  AttribSpec(int i, int v) : index(i), val(v) {}
  int index;
  int val;
};

struct QueryTest
{
  QueryTest(const std::vector<AttribSpec> & query, size_t wantn, const std::vector<int> & wantvals)
    : query(query), wantn(wantn), wantvals(wantvals)
  {
  }
  // a list of desired attribute values to build a warehouse query from.
  std::vector<AttribSpec> query;
  // the number of expected results from running the given query.
  size_t wantn;
  // A pattern of attribute values that each object should conform to (i.e. four ints representing
  // attribute values of resulting objects from query). Negative values are not compared (i.e. are
  // wildcards).
  std::vector<int> wantvals;
};

struct WarehouseTest
{
  // a vector of four ints for each object to add to the warehouse.  The ints specify the attrib
  // values for TestObject instances.
  std::vector<std::vector<int>> objects;
  // A list of queryies (and expected results) to run after objects have been added to the
  // warehouse.
  std::vector<QueryTest> queries;
};

TEST_F(TheWarehouseTest, hashing)
{
  std::vector<std::unique_ptr<Attribute>> attribs1;
  attribs1.emplace_back(new TestAttrib(w, 0, 42));
  attribs1.emplace_back(new TestAttrib(w, 1, 43));
  std::hash<std::vector<std::unique_ptr<Attribute>>> hasher{};
  auto h1 = hasher(attribs1);
  auto h2 = hasher(attribs1);

  std::vector<std::unique_ptr<Attribute>> attribs2;
  attribs2.emplace_back(new TestAttrib(w, 0, 42));
  attribs2.emplace_back(new TestAttrib(w, 1, 43));
  auto h3 = hasher(attribs2);
  auto h4 = hasher(attribs2);

  EXPECT_EQ(h1, h2);
  EXPECT_EQ(h2, h3);
  EXPECT_EQ(h3, h4);

  std::unordered_map<std::vector<std::unique_ptr<Attribute>>, int> cache;
  cache[std::move(attribs1)] = 1;

  auto it = cache.find(attribs2);
  EXPECT_TRUE(it != cache.end());
  EXPECT_EQ(cache.count(attribs2), 1);
}

TEST_F(TheWarehouseTest, benchmark)
{
  bool run = false;
  // run = true;
  if (!run)
    return;

  int n_objs = 1000;
  int n_queries = 2000;
  int n_repeat_query = 20000;
  for (int i = 0; i < n_objs; i++)
    w.add(obj(i % 1000, 1, 1, 1));

  int cum_results = 0;
  for (int n = 0; n < n_repeat_query; n++)
  {
    for (int i = 0; i < n_queries; i++)
    {
      std::vector<TestObject *> objs;
      w.query()
          .condition<TestAttrib>(0, i % 1000)
          .condition<TestAttrib>(1, 1)
          .condition<TestAttrib>(2, 1)
          .condition<TestAttrib>(2, 1)
          .queryInto(objs);
      cum_results += objs.size();
    }
  }
  Moose::out << "ran " << n_queries << " unique queries each repeated " << n_repeat_query
             << " times over " << n_objs << " objects totaling " << cum_results
             << " resulting objects\n";
}

// A series of warehouse querying tests - each test has a set of objects that get added to the
// warehouse
TEST_F(TheWarehouseTest, test)
{
  // clang-format off
  std::vector<WarehouseTest> cases = {
      WarehouseTest{
          {
              {1, 1, 1, 1},
              {1, 1, 2, 1},
              {1, 1, 2, 2},
              {1, 1, 1, 3},
          },{
              QueryTest({AttribSpec(0, 0)                  }, 0, {-1, -1, -1, -1}),
              QueryTest({AttribSpec(0, 1)                  }, 4, { 1, -1, -1, -1}),
              QueryTest({AttribSpec(2, 1)                  }, 2, {-1, -1,  1, -1}),
              QueryTest({AttribSpec(2, 2)                  }, 2, {-1, -1,  2, -1}),
              QueryTest({AttribSpec(0, 1), AttribSpec(1, 1)}, 4, { 1,  1, -1, -1}),
              QueryTest({AttribSpec(2, 2), AttribSpec(3, 2)}, 1, {-1, -1,  2,  2}),
              QueryTest({AttribSpec(0, 1), AttribSpec(3, 3)}, 1, { 1, -1, -1,  3}),
          }
      },
  };
  // clang-format on

  bool totalfail = false;
  for (size_t i = 0; i < cases.size(); i++)
  {
    std::stringstream msg;
    msg << "---------- case " << i + 1 << " ----------\n";
    auto & test = cases[i];

    msg << "objects:\n";
    for (auto & vals : test.objects)
    {
      msg << "    * {";
      for (auto val : vals)
        msg << val << ",";
      msg << "}\n";
      w.add(obj(vals[0], vals[1], vals[2], vals[3]));
    }

    for (size_t j = 0; j < test.queries.size(); j++)
    {
      auto query_test = test.queries[j];
      auto query = w.query();
      msg << "* query " << j + 1 << " { ";
      for (auto & attrib : query_test.query)
      {
        msg << "{val_" << attrib.index + 1 << " = " << attrib.val << "}, ";
        query.condition<TestAttrib>(attrib.index, attrib.val);
      }
      msg << "}: ";

      std::vector<TestObject *> objs;
      query.queryInto(objs);

      std::stringstream qmsg; // only printed if this query test fails
      qmsg << "FAIL\n";
      bool failed = false;
      if (query_test.wantn != objs.size())
      {
        qmsg << "    * wrong number results: want " << query_test.wantn << ", got " << objs.size()
             << "\n";
        failed = true;
      }

      if (objs.size() > 0)
      {
        qmsg << "    * checking objects - want vals {";
        for (auto val : query_test.wantvals)
          qmsg << val << ",";
        qmsg << "}:\n";
      }

      for (size_t k = 0; k < objs.size(); k++)
      {
        qmsg << "        * object " << k + 1 << " {";
        for (auto val : objs[k]->vals)
          qmsg << val << ",";
        qmsg << "}: ";

        bool failvals = false;
        if (query_test.wantvals[0] >= 0 && query_test.wantvals[0] != objs[k]->vals[0])
        {
          failvals = true;
          qmsg << " index 1 FAIL, ";
        }
        if (query_test.wantvals[1] >= 0 && query_test.wantvals[1] != objs[k]->vals[1])
        {
          failvals = true;
          qmsg << " index 2 FAIL, ";
        }
        if (query_test.wantvals[2] >= 0 && query_test.wantvals[2] != objs[k]->vals[2])
        {
          failvals = true;
          qmsg << " index 3 FAIL, ";
        }
        if (query_test.wantvals[3] >= 0 && query_test.wantvals[3] != objs[k]->vals[3])
        {
          failvals = true;
          qmsg << " index 4 FAIL, ";
        }
        failed |= failvals;
        if (!failvals)
          qmsg << " PASS";
        qmsg << "\n";
      }
      if (failed)
      {
        Moose::out << msg.str() << qmsg.str();
        msg.str("");
      }
      else
        msg << "PASS\n";
      totalfail |= failed;
    }
  }
  if (totalfail)
    FAIL();
}
