#ifndef UTILS_H
#define UTILS_H

#define REL_TEST(name, value, ref_value, tol)                                                      \
  EXPECT_LE(std::abs((value) - (ref_value)) / (ref_value), (tol)) << "    - failed " << name       \
                                                                  << " check"

#define ABS_TEST(name, value, ref_value, tol)                                                      \
  EXPECT_LE(std::abs(((value) - (ref_value))), (tol)) << "    - failed " << name << " check"

#endif // UTILS_H
