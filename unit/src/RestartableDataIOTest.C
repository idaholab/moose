//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "RestartableDataIOTest.h"

#include "AppFactory.h"
#include "MooseApp.h"
#include "RestartableDataReader.h"
#include "RestartableDataWriter.h"

#include "libmesh/int_range.h"

#include <memory>

void
RestartableDataIOTest::SetUp()
{
  const char * argv[2] = {"foo", "\0"};
  _app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
}

std::map<std::string, RestartableDataIOTest::DataInfo>
basicData()
{
  std::map<std::string, RestartableDataIOTest::DataInfo> data;
  bool every_other = true;
  for (const auto i : make_range(unsigned(10)))
  {
    const std::string name = "value" + std::to_string(i);
    auto & val = data[name];
    val.value = 100 + 2 * i;
    val.no_declare = every_other;

    every_other = !every_other;
  }

  return data;
}

TEST_F(RestartableDataIOTest, readWrite)
{
  const auto data = basicData();

  // For use in a test with context
  const std::string with_context_name = "with_context";
  const unsigned int dummy_context = 0;

  // Streams we're going to use on both sides
  auto header_stream = std::make_unique<std::stringstream>();
  auto data_stream = std::make_unique<std::stringstream>();
  {
    // Fill the map that we're storing
    RestartableDataMap rdm;
    for (const auto & [name, info] : data)
    {
      std::unique_ptr<RestartableDataValue> rdv =
          std::make_unique<RestartableData<unsigned int>>(name, nullptr, info.value);
      rdm.addData(std::move(rdv));
    }

    // Shouldn't be stored or loaded
    for (const auto & val : rdm)
    {
      EXPECT_FALSE(val.stored());
      EXPECT_FALSE(val.loaded());
    }

    // Declare a value with context for use later
    {
      std::unique_ptr<RestartableDataValue> rdv =
          std::make_unique<RestartableData<unsigned int>>(with_context_name, &dummy_context);
      rdm.addData(std::move(rdv));
    }

    // And write
    RestartableDataWriter writer(*_app, rdm);
    writer.write(*header_stream, *data_stream);

    // Should be stored and not loaded
    for (const auto & val : rdm)
    {
      EXPECT_TRUE(val.stored());
      EXPECT_FALSE(val.loaded());
    }
  }

  // Declare some of the values
  RestartableDataMap rdm;
  for (const auto & [name, info] : data)
    if (!info.no_declare)
    {
      std::unique_ptr<RestartableDataValue> rdv =
          std::make_unique<RestartableData<unsigned int>>(name, nullptr, 0);
      EXPECT_FALSE(rdv->stored());
      EXPECT_FALSE(rdv->loaded());
      rdm.addData(std::move(rdv));
    }

  // Do the initial load
  RestartableDataReader reader(*_app, rdm);
  reader.setInput(std::move(header_stream), std::move(data_stream));
  reader.restore();
  auto & late_restorer = reader.getLateRestorer();

  // Make sure every other value is there and has the right value
  for (const auto & [name, info] : data)
  {
    EXPECT_EQ(info.no_declare, late_restorer.isRestorable<unsigned int>(name));

    if (!info.no_declare)
    {
      const auto & rdv = rdm.data(name);
      auto rdv_uint = dynamic_cast<const RestartableData<unsigned int> *>(&rdv);
      EXPECT_TRUE(rdv_uint != nullptr);
      EXPECT_EQ(rdv_uint->get(), info.value);
      EXPECT_FALSE(rdv.stored());
      EXPECT_TRUE(rdv.loaded());

      try
      {
        late_restorer.restore(name);
        FAIL();
      }
      catch (const std::exception & err)
      {
        const auto pos = std::string(err.what()).find("already been loaded");
        ASSERT_TRUE(pos != std::string::npos);
      }
    }
  }

  // Try loading later
  for (const auto & [name, info] : data)
    if (info.no_declare)
    {
      try
      {
        rdm.data(name);
        FAIL();
      }
      catch (const std::exception & err)
      {
        const auto pos = std::string(err.what()).find("not registered");
        ASSERT_TRUE(pos != std::string::npos);
      }

      try
      {
        late_restorer.restore(name);
        FAIL();
      }
      catch (const std::exception & err)
      {
        const auto pos = std::string(err.what()).find("not been declared");
        ASSERT_TRUE(pos != std::string::npos);
      }

      std::unique_ptr<RestartableDataValue> rdv =
          std::make_unique<RestartableData<unsigned int>>(name, nullptr);
      const auto & val = dynamic_cast<RestartableData<unsigned int> &>(*rdv).get();
      rdm.addData(std::move(rdv));

      EXPECT_FALSE(val == info.value);
      late_restorer.restore(name);
      EXPECT_EQ(val, info.value);
    }

  // Try loading later with a context
  {
    std::unique_ptr<RestartableDataValue> rdv =
        std::make_unique<RestartableData<unsigned int>>(with_context_name, nullptr);
    rdm.addData(std::move(rdv));
  }
  try
  {
    late_restorer.restore(with_context_name);
    FAIL();
  }
  catch (const std::exception & err)
  {
    const auto pos = std::string(err.what()).find("requires a context");
    ASSERT_TRUE(pos != std::string::npos);
  }

  // Clear and make sure we can't do this anymore
  reader.clear();
}

TEST_F(RestartableDataIOTest, readerErrors)
{
  RestartableDataMap rdm;
  RestartableDataReader reader(*_app, rdm);

  try
  {
    reader.restore();
    FAIL();
  }
  catch (const std::exception & err)
  {
    const auto pos = std::string(err.what()).find("input was not set");
    ASSERT_TRUE(pos != std::string::npos);
  }
}
