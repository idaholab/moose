#ifdef MOOSE_LIBTORCH_ENABLED

#include "gtest/gtest.h"
#include "Standardizer.h"

#include <sstream>
#include <torch/torch.h>

using namespace StochasticTools;

TEST(StochasticTools, getMean)
{
  const std::vector<double> mean_gold = {0.0, -1.0};
  const std::vector<double> stddev_gold = {1.0, 0.0};

  torch::Tensor input_tensor = torch::tensor(
      {{1.0, -1.0}, {-1.0, -1.0}, {1.0, -1.0}, {-1.0, -1.0}, {1.0, -1.0}, {-1.0, -1.0}},
      {torch::kFloat64});

  StochasticTools::Standardizer standardizer;
  standardizer.computeSet(input_tensor);

  const auto mean = standardizer.getMean();
  const auto stddev = standardizer.getStdDev();

  ASSERT_EQ(mean.dim(), 1);
  ASSERT_EQ(stddev.dim(), 1);
  ASSERT_EQ(static_cast<std::size_t>(mean.size(0)), mean_gold.size());
  ASSERT_EQ(static_cast<std::size_t>(stddev.size(0)), stddev_gold.size());

  const auto mean_accessor = mean.accessor<Real, 1>();
  const auto stddev_accessor = stddev.accessor<Real, 1>();
  for (std::size_t i = 0; i < mean_gold.size(); ++i)
    EXPECT_EQ(mean_accessor[i], mean_gold[i]);
  for (std::size_t i = 0; i < stddev_gold.size(); ++i)
    EXPECT_EQ(stddev_accessor[i], stddev_gold[i]);
}

TEST(StochasticTools, getStandardized)
{
  torch::Tensor input_tensor = torch::tensor({{1.0, -1.0}, {-1.0, 1.0}}, {torch::kFloat64});
  const auto gold = torch::tensor({{1.0, -1.0}, {-1.0, 1.0}}, {torch::kFloat64});

  StochasticTools::Standardizer standardizer;
  standardizer.computeSet(input_tensor);
  standardizer.getStandardized(input_tensor);

  EXPECT_TRUE(torch::allclose(input_tensor, gold));
}

TEST(StochasticTools, getDestandardized)
{
  torch::Tensor input_tensor = torch::tensor({{1.0, -1.0}, {-1.0, 1.0}}, {torch::kFloat64});
  const auto gold = torch::tensor({{1.0, -1.0}, {-1.0, 1.0}}, {torch::kFloat64});

  StochasticTools::Standardizer standardizer;
  standardizer.computeSet(input_tensor);
  standardizer.getStandardized(input_tensor);
  standardizer.getDestandardized(input_tensor);

  EXPECT_TRUE(torch::allclose(input_tensor, gold));
}

TEST(StochasticTools, getDescaled)
{
  torch::Tensor input_tensor = torch::tensor({{1.0, -1.0}, {1.0, 1.0}}, {torch::kFloat64});
  const auto gold = torch::tensor({{0.0, -1.0}, {0.0, 1.0}}, {torch::kFloat64});

  StochasticTools::Standardizer standardizer;
  standardizer.computeSet(input_tensor);
  standardizer.getDescaled(input_tensor);

  EXPECT_TRUE(torch::allclose(input_tensor, gold));
}

TEST(StochasticTools, getScaled)
{
  torch::Tensor input_tensor = torch::tensor({{4.0, -8.0}, {-4.0, 8.0}}, {torch::kFloat64});
  const auto reference = torch::tensor({{2.0, -4.0}, {-2.0, 4.0}}, {torch::kFloat64});
  const auto gold = torch::tensor({{2.0, -2.0}, {-2.0, 2.0}}, {torch::kFloat64});

  StochasticTools::Standardizer standardizer;
  standardizer.computeSet(reference);
  standardizer.getScaled(input_tensor);

  EXPECT_TRUE(torch::allclose(input_tensor, gold));
}

TEST(StochasticTools, tensorDataStoreLoad)
{
  torch::Tensor stored = torch::tensor({{1.0, 2.0, 3.0}, {-4.0, -5.0, -6.0}}, {torch::kFloat64});

  std::stringbuf buffer;
  std::iostream stream(&buffer);
  dataStore(stream, stored, nullptr);

  torch::Tensor loaded;
  dataLoad(stream, loaded, nullptr);

  ASSERT_EQ(loaded.size(0), stored.size(0));
  ASSERT_EQ(loaded.size(1), stored.size(1));
  EXPECT_TRUE(torch::allclose(loaded, stored));
}

TEST(StochasticTools, tensorScalarDataStoreLoad)
{
  torch::Tensor stored = torch::tensor(3.25, {torch::kFloat64});

  std::stringbuf buffer;
  std::iostream stream(&buffer);
  dataStore(stream, stored, nullptr);

  torch::Tensor loaded;
  dataLoad(stream, loaded, nullptr);

  ASSERT_EQ(loaded.dim(), stored.dim());
  EXPECT_TRUE(torch::allclose(loaded, stored));
}

TEST(StochasticTools, tensorVectorDataStoreLoad)
{
  torch::Tensor stored = torch::tensor({1.0, -2.0, 3.5, 7.0}, {torch::kFloat64});

  std::stringbuf buffer;
  std::iostream stream(&buffer);
  dataStore(stream, stored, nullptr);

  torch::Tensor loaded;
  dataLoad(stream, loaded, nullptr);

  ASSERT_EQ(loaded.dim(), stored.dim());
  ASSERT_EQ(loaded.size(0), stored.size(0));
  EXPECT_TRUE(torch::allclose(loaded, stored));
}

TEST(StochasticTools, tensorDataStoreLoadNonContiguous)
{
  const torch::Tensor base =
      torch::tensor({{1.0, 2.0, 3.0}, {-4.0, -5.0, -6.0}}, {torch::kFloat64});
  torch::Tensor stored = torch::transpose(base, 0, 1);

  ASSERT_FALSE(stored.is_contiguous());

  std::stringbuf buffer;
  std::iostream stream(&buffer);
  dataStore(stream, stored, nullptr);

  torch::Tensor loaded;
  dataLoad(stream, loaded, nullptr);

  ASSERT_EQ(loaded.size(0), stored.size(0));
  ASSERT_EQ(loaded.size(1), stored.size(1));
  EXPECT_TRUE(torch::allclose(loaded, stored));
}

TEST(StochasticTools, standardizerDataStoreLoad)
{
  torch::Tensor input_tensor =
      torch::tensor({{3.0, 1.0}, {5.0, -1.0}, {7.0, 3.0}}, {torch::kFloat64});

  StochasticTools::Standardizer stored;
  stored.computeSet(input_tensor);

  std::stringbuf buffer;
  std::iostream stream(&buffer);
  dataStore(stream, stored, nullptr);

  StochasticTools::Standardizer loaded;
  dataLoad(stream, loaded, nullptr);

  EXPECT_TRUE(torch::allclose(loaded.getMean(), stored.getMean()));
  EXPECT_TRUE(torch::allclose(loaded.getStdDev(), stored.getStdDev()));
}

TEST(StochasticTools, standardizerEmptyDataStoreLoad)
{
  StochasticTools::Standardizer stored;
  std::stringbuf buffer;
  std::iostream stream(&buffer);
  dataStore(stream, stored, nullptr);
  StochasticTools::Standardizer loaded;
  dataLoad(stream, loaded, nullptr);
}

#endif
