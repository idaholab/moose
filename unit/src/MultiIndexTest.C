/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

//Magpie includes
#include "MultiIndex.h"

//libMesh include
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

#include <gtest/gtest.h>

TEST(MultiIndexTest, setUp)
{
  // Construct a 3 indexed objects of Reals
  MultiIndex<Real>::size_type shape(3);
  shape[0] = 3;
  shape[1] = 2;
  shape[2] = 4;
  MultiIndex<Real> mindex1 = MultiIndex<Real>(shape);

  // check dimension
  EXPECT_EQ(mindex1.dim(), 3);

  // check size operator
  for (unsigned int j = 0; j < 3; ++j)
  EXPECT_EQ(mindex1.size()[j], shape[j]);

  // parenthesis operator
  MultiIndex<Real>::size_type index(3);
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
      {
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        mindex1(index) = j0 + 10.0 * j1 + 100.0 * j2;
      }

  // check the parenthesis operator but this time reverse loop order
  for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
    for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
      for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      {
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        EXPECT_TRUE(mindex1(index) == j0 + 10.0 * j1 + 100.0 * j2);
      }

  // check the two-argument constructor
  std::vector<Real> data(mindex1.nEntries());
  unsigned int p = 0;
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
      {
        data[p] = j0 - 3.0 * j1 + 100.0 * j2;
        p++;
      }

  MultiIndex<Real> mindex2 = MultiIndex<Real>(shape, data);
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
      {
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        EXPECT_TRUE(mindex2(index) == j0 - 3.0 * j1 + 100.0 * j2);
      }

  // test resize increasing and descreasing in different dimension simultaneously
  MultiIndex<Real>::size_type new_shape(3);
  new_shape[0] = 2;
  new_shape[1] = 3;
  new_shape[2] = 5;
  mindex2.resize(new_shape);
  for (unsigned int j0 = 0; j0 < new_shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < new_shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < new_shape[2]; ++j2)
      {
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        if (j0 < shape[0] && j1 < shape[1] && j2 < shape[2])
        EXPECT_TRUE(mindex2(index) == j0 - 3.0 * j1 + 100.0 * j2);
        else
        EXPECT_EQ(mindex2(index), 0.0);
      }

      // test assign increasing and descreasing in different dimension simultaneously
      index.resize(4);
      new_shape.resize(4);
      new_shape[0] = 4;
      new_shape[1] = 2;
      new_shape[2] = 1;
      new_shape[3] = 8;
      mindex2.assign(new_shape, 1.0);
      for (unsigned int j0 = 0; j0 < new_shape[0]; ++j0)
        for (unsigned int j1 = 0; j1 < new_shape[1]; ++j1)
          for (unsigned int j2 = 0; j2 < new_shape[2]; ++j2)
            for (unsigned int j3 = 0; j3 < new_shape[3]; ++j3)
            {
              index[0] = j0;
              index[1] = j1;
              index[2] = j2;
              index[3] = j3;
              EXPECT_EQ(mindex2(index), 1.0);
            }

}

TEST(MultiIndexTest, testIterators)
{
  // Create empty MultiIndex object
  MultiIndex<Real>::size_type shape(3);
  shape[0] = 3;
  shape[1] = 2;
  shape[2] = 4;
  MultiIndex<Real> mindex = MultiIndex<Real>(shape);

  // set the data by using an iterator
  auto it = mindex.begin();
  auto it_end = mindex.end();
  for (; it != it_end ; ++it)
    (*it).second = (*it).first[0] - 3.0 * (*it).first[1] + 100.0 * (*it).first[2];

  // check the values
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
      {
        MultiIndex<Real>::size_type index(mindex.dim());
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        EXPECT_TRUE(mindex(index) == j0 - 3.0 * j1 + 100.0 * j2);
      }

  // check the indices function
  it = mindex.begin();
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
      {
        MultiIndex<Real>::size_type index(mindex.dim());
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        EXPECT_EQ(index[0], (*it).first[0]);
        EXPECT_EQ(index[1], (*it).first[1]);
        EXPECT_EQ(index[2], (*it).first[2]);
        ++it;
      }

  // test the decrement operator
  it = mindex.begin();
  it_end = mindex.end();
  while (it != it_end)
  {
    auto indices = (*it).first;
    (*it).second = indices[0] - 7.0 * indices[1] + 100.0 * indices[2];
    ++it;++it;--it;
  }

  // check the values
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
      {
        std::vector<unsigned int> index(mindex.dim());
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        EXPECT_TRUE(mindex(index) == j0 - 7.0 * j1 + 100.0 * j2);
      }
}

TEST(MultiIndexTest, dataStoreLoad)
{
  // Create empty MultiIndex object
  MultiIndex<Real>::size_type shape(3);
  shape[0] = 3;
  shape[1] = 2;
  shape[2] = 4;
  MultiIndex<Real> mindex = MultiIndex<Real>(shape);

  // set the data by using an iterator
  auto it = mindex.begin();
  auto it_end = mindex.end();
  for (; it != it_end ; ++it)
    (*it).second = (*it).first[0] - 3.0 * (*it).first[1] + 100.0 * (*it).first[2];

  // Serialize
  std::ostringstream oss;
  dataStore(oss, mindex, this);

  // Pour oss into iss
  std::string ostring = oss.str();
  std::istringstream iss(ostring);

  // Clear data structure to avoid false positives and then
  // read data
  for (it = mindex.begin(); it != it_end ; ++it)
    (*it).second = 0.0;

  dataLoad(iss, mindex, this);

  // check the values
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
      {
        MultiIndex<Real>::size_type index(mindex.dim());
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        EXPECT_TRUE(mindex(index) == j0 - 3.0 * j1 + 100.0 * j2);
      }
}

TEST(MultiIndexTest, slice)
{
  // Create empty MultiIndex object
  MultiIndex<unsigned int>::size_type shape(4);
  shape[0] = 3;
  shape[1] = 5;
  shape[2] = 4;
  shape[3] = 6;
  MultiIndex<unsigned int> mindex = MultiIndex<unsigned int>(shape);

  // set the data by using an iterator
  MultiIndex<unsigned int>::iterator it = mindex.begin();
  MultiIndex<unsigned int>::iterator it_end = mindex.end();
  for (; it != it_end ; ++it)
  {
    MultiIndex<unsigned int>::size_type indices = (*it).first;
    (*it).second = indices[0] + 10 * indices[1] + 100 * indices[2] + 1000 * indices[3];
  }

  // slice multi index at dim = 1, index = 3
  MultiIndex<unsigned int> sliced_mindex = mindex.slice(1, 3);

  // check the values
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
        for (unsigned int j3 = 0; j3 < shape[3]; ++j3)
        {
          if (j1 == 3)
          {
            MultiIndex<unsigned int>::size_type index(sliced_mindex.dim());
            index[0] = j0;
            index[1] = j2;
            index[2] = j3;
            EXPECT_TRUE(sliced_mindex(index) == j0 + 10 * j1 + 100 * j2 + 1000 * j3);
          }
        }

  // test the vector version of this functions
  std::vector<unsigned int> vec_dim(1);
  std::vector<unsigned int> vec_ind(1);
  vec_dim[0] = 1;
  vec_ind[0] = 3;
  MultiIndex<unsigned int> sliced_mindex_vec = mindex.slice(vec_dim, vec_ind);
  // check the values
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
        for (unsigned int j3 = 0; j3 < shape[3]; ++j3)
        {
          if (j1 == 3)
          {
            MultiIndex<unsigned int>::size_type index(sliced_mindex_vec.dim());
            index[0] = j0;
            index[1] = j2;
            index[2] = j3;
            EXPECT_TRUE(sliced_mindex_vec(index) == j0 + 10 * j1 + 100 * j2 + 1000 * j3);
          }
        }

  // test the vector version of this functions and slice for two indices
  vec_dim.resize(2);
  vec_ind.resize(2);
  vec_dim[0] = 1;
  vec_dim[1] = 3;
  vec_ind[0] = 3;
  vec_ind[1] = 2;
  MultiIndex<unsigned int> sliced_mindex_vec2 = mindex.slice(vec_dim, vec_ind);
  // check the values
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
        for (unsigned int j3 = 0; j3 < shape[3]; ++j3)
        {
          if (j1 == 3 && j3 == 2)
          {
            MultiIndex<unsigned int>::size_type index(sliced_mindex_vec2.dim());
            index[0] = j0;
            index[1] = j2;
            EXPECT_TRUE(sliced_mindex_vec2(index) == j0 + 10 * j1 + 100 * j2 + 1000 * j3);
          }
        }
}
