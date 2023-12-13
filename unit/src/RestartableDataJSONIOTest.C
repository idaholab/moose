//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "nlohmann/json.h"

#include "RestartableData.h"

TEST(RestartableDataJSONIOTest, store)
{
  RestartableData<unsigned int> data("cool_data", nullptr, 5);

  const auto test_values =
      [](const auto & data, const RestartableDataValue::StoreJSONParams & params)
  {
    nlohmann::json json;
    data.store(json, params);

    EXPECT_EQ(json.contains("value"), params.value);
    if (params.value)
    {
      EXPECT_EQ(json.at("value"), data.get());
    }

    EXPECT_EQ(json.contains("type"), params.type);
    if (params.type)
    {
      EXPECT_EQ(json.at("type"), data.type());
    }

    EXPECT_EQ(json.contains("name"), params.name);
    if (params.name)
    {
      EXPECT_EQ(json.at("name"), data.name());
    }

    EXPECT_EQ(json.contains("declared"), params.declared);
    if (params.declared)
    {
      EXPECT_EQ(json.at("declared"), data.declared());
    }

    EXPECT_EQ(json.contains("loaded"), params.loaded);
    if (params.loaded)
    {
      EXPECT_EQ(json.at("loaded"), data.loaded());
    }

    EXPECT_EQ(json.contains("stored"), params.stored);
    if (params.stored)
    {
      EXPECT_EQ(json.at("stored"), data.stored());
    }

    EXPECT_EQ(json.contains("has_context"), params.has_context);
    if (params.has_context)
    {
      EXPECT_EQ(json.at("has_context"), data.hasContext());
    }
  };

  // Should have a to_json
  EXPECT_TRUE(data.has_store_json);
  EXPECT_TRUE(data.hasStoreJSON());

  RestartableDataValue::StoreJSONParams params;
  // Default parameters
  test_values(data, params);
  // Remove value
  params.value = false;
  test_values(data, params);
  // Remove type
  params.type = false;
  test_values(data, params);
  // Add name
  params.name = true;
  test_values(data, params);
  // Add declared
  params.declared = true;
  test_values(data, params);
  // Add loaded
  params.loaded = true;
  test_values(data, params);
  // Add stored
  params.stored = true;
  test_values(data, params);
  // Add has_context
  params.has_context = true;
  test_values(data, params);
}

struct CustomData
{
  std::string some_string;
  unsigned int some_int;
};

void
dataStore(std::ostream &, CustomData &, void *)
{
}

void
dataLoad(std::ostream &, CustomData &, void *)
{
}

TEST(RestartableDataJSONIOTest, missingStoreJSON)
{
  RestartableData<CustomData> data("cooler_data", nullptr);

  EXPECT_FALSE(data.has_store_json);
  EXPECT_FALSE(data.hasStoreJSON());

  // Store with the value by default, which should error (no to_json)
  try
  {
    nlohmann::json json;
    data.store(json);
    FAIL();
  }
  catch (const std::exception & err)
  {
    const auto pos = std::string(err.what()).find("Failed to output restartable data");
    EXPECT_TRUE(pos != std::string::npos);
  }

  // Should be able to store without the value
  RestartableDataValue::StoreJSONParams params;
  params.value = false;
  nlohmann::json json;
  data.store(json, params);
  EXPECT_EQ(json.at("type"), data.type());
}
