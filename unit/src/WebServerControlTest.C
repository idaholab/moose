//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "WebServerControl.h"
#include "nlohmann/json.h"

TEST(WebServerControl, toMiniJsonFromNlohmann)
{
  nlohmann::json json;

  {
    json["pi"] = 3.14;
    const auto mini_json_value = WebServerControl::toMiniJson(json["pi"]);
    EXPECT_TRUE(mini_json_value.isNumber());
    EXPECT_NEAR(mini_json_value.toDouble(), 3.14, 1e-12);
  }

  {
    json["happy"] = true;
    const auto mini_json_value = WebServerControl::toMiniJson(json["happy"]);
    EXPECT_TRUE(mini_json_value.isBool());
    EXPECT_TRUE(mini_json_value.toBool());
  }

  {
    json["name"] = "Zach";
    const auto mini_json_value = WebServerControl::toMiniJson(json["name"]);
    EXPECT_TRUE(mini_json_value.isString());
    EXPECT_EQ(mini_json_value.toString(), "Zach");
  }

  {
    json["year"] = 2025;
    const auto mini_json_value = WebServerControl::toMiniJson(json["year"]);
    EXPECT_TRUE(mini_json_value.isNumber());
    EXPECT_EQ((int)mini_json_value.toDouble(), 2025);
  }

  {
    std::vector<Real> vector = {0.1, 1.2, 2.3};
    json["vector"] = vector;
    const auto mini_json_value = WebServerControl::toMiniJson(json["vector"]);
    EXPECT_TRUE(mini_json_value.isArray());
    const auto array_value = mini_json_value.toArray();
    EXPECT_EQ(array_value.size(), vector.size());
    for (unsigned int i = 0; i < vector.size(); ++i)
    {
      EXPECT_TRUE(array_value[i].isNumber());
      EXPECT_NEAR(array_value[i].toDouble(), vector[i], 1e-12);
    }
  }
}
