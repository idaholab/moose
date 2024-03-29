//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "DataIO.h"

struct DataType
{
  unsigned int data;
};

struct DataStorage : public UniqueStorage<DataType>
{
  FRIEND_TEST(DataIOTest, uniqueStorage);
};

void
dataStore(std::ostream & stream, DataType & v, void * context)
{
  dataStore(stream, v.data, context);
}

void
dataLoad(std::istream & stream, DataType & v, void * context)
{
  dataLoad(stream, v.data, context);
}

void
dataStore(std::ostream & stream, std::unique_ptr<DataType> & v, void * context)
{
  dataStore(stream, *v, context);
}

void
dataLoad(std::istream & stream, std::unique_ptr<DataType> & v, void * context)
{
  v = std::make_unique<DataType>();
  dataLoad(stream, *v, context);
}

void
dataStore(std::ostream & stream, DataStorage & v, void * context)
{
  storeHelper(stream, static_cast<UniqueStorage<DataType> &>(v), context);
}

void
dataLoad(std::istream & stream, DataStorage & v, void * context)
{
  loadHelper(stream, static_cast<UniqueStorage<DataType> &>(v), context);
}

TEST(DataIOTest, uniqueStorage)
{
  const std::vector<unsigned int> data = {0, 5, 4};
  DataStorage storage;
  for (auto val : data)
    storage.addPointer(std::make_unique<DataType>()).data = val;

  std::stringstream ss;
  dataStore(ss, storage, nullptr);

  ss.seekg(0, std::ios::beg);
  DataStorage loaded_storage;
  dataLoad(ss, loaded_storage, nullptr);

  ASSERT_EQ(storage.size(), loaded_storage.size());
  for (const auto i : index_range(storage))
    ASSERT_EQ(storage[i].data, loaded_storage[i].data);
}

TEST(DataIOTest, uniquePtrNumericVector)
{
  libMesh::Parallel::Communicator comm(MPI_COMM_WORLD);

  const std::vector<Real> data = {1.1, 1.5, 1e6};
  auto vec = NumericVector<Number>::build(comm);
  vec->init(data.size());
  for (const auto i : index_range(data))
    vec->set(i, data[i]);

  std::stringstream ss;
  dataStore(ss, vec, &comm);

  // Construct new
  ss.seekg(0, std::ios::beg);
  std::unique_ptr<NumericVector<Number>> new_vec;
  dataLoad(ss, new_vec, &comm);
  ASSERT_EQ(new_vec->size(), vec->size());
  ASSERT_EQ(new_vec->local_size(), vec->local_size());
  for (const auto i : index_range(data))
    ASSERT_EQ((*new_vec)(i), data[i]);

  // Change the data
  new_vec->add(1);
  new_vec->close();
  for (const auto i : index_range(data))
    ASSERT_NE((*new_vec)(i), data[i]);

  // Reload again
  const auto new_vec_ptr = new_vec.get();
  ss.seekg(0, std::ios::beg);
  dataLoad(ss, new_vec, &comm);
  ASSERT_EQ(new_vec_ptr, new_vec.get());
  ASSERT_EQ(new_vec->size(), vec->size());
  ASSERT_EQ(new_vec->local_size(), vec->local_size());
  for (const auto i : index_range(data))
    ASSERT_EQ((*new_vec)(i), data[i]);
}
