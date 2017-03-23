#ifndef UTILS_H
#define UTILS_H

#define REL_TEST(name, value, ref_value, tol)                                                      \
  CPPUNIT_ASSERT((std::abs(((value) - (ref_value)) / (ref_value))) <= (tol));

#define ABS_TEST(name, value, ref_value, tol)                                                      \
  CPPUNIT_ASSERT((std::abs(((value) - (ref_value)))) <= (tol));

#endif // UTILS_H
