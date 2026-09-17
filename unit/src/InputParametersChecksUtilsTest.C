//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObjectUnitTest.h"
#include "InputParametersChecksUtils.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"

/// Minimal object that mixes in InputParametersChecksUtils so its protected checks
/// can be exercised. It declares parameters of every type the checks operate on;
/// a parameter is considered "set by the user" only when its value is provided to
/// the factory. A public wrapper is provided for each protected check.
///
/// blocks() is defined so that the block-restriction based check can be exercised
/// without having to build a full BlockRestrictable object; _blocks is set directly
/// by the tests to control the reported block restriction.
class CheckParamsTestObject : public MooseObject,
                              public InputParametersChecksUtils<CheckParamsTestObject>
{
public:
  static InputParameters validParams();

  CheckParamsTestObject(const InputParameters & params)
    : MooseObject(params), InputParametersChecksUtils<CheckParamsTestObject>(this)
  {
  }

  /// Block restriction returned to the checks utility (settable by the tests)
  const std::vector<SubdomainName> & blocks() const { return _blocks; }
  std::vector<SubdomainName> _blocks;

  // --- Public wrappers around each protected check under test ---
  template <typename T>
  void wrapAssertParamDefined(const std::string & p) const
  {
    assertParamDefined<T>(p);
  }
  void wrapBothSetOrNotSet(const std::string & p1, const std::string & p2) const
  {
    checkParamsBothSetOrNotSet(p1, p2);
  }
  void wrapAtMostOne(const std::vector<std::string> & p) const { checkAtMostOneParamSetByUser(p); }
  void wrapSecondOnlyIfTrue(const std::string & p1, const std::string & p2) const
  {
    checkSecondParamSetOnlyIfFirstOneTrue(p1, p2);
  }
  void wrapSecondOnlyIfSet(const std::string & p1, const std::string & p2) const
  {
    checkSecondParamSetOnlyIfFirstOneSet(p1, p2);
  }
  void wrapSecondNotIfSet(const std::string & p1, const std::string & p2) const
  {
    checkSecondParamNotSetIfFirstOneSet(p1, p2);
  }
  void wrapVecSameLength(const std::string & p1, const std::string & p2) const
  {
    checkVectorParamsSameLength<Real, Real>(p1, p2);
  }
  void wrapVecMMELength(const std::string & p1, const std::string & p2) const
  {
    checkVectorParamAndMultiMooseEnumLength<Real>(p1, p2);
  }
  void wrapTwoDSameLength(const std::string & p1, const std::string & p2) const
  {
    checkTwoDVectorParamsSameLength<Real, Real>(p1, p2);
  }
  void wrapVecNoOverlap(const std::vector<std::string> & p) const
  {
    checkVectorParamsNoOverlap<std::string>(p);
  }
  void wrapTwoDNoOverlap(const std::vector<std::string> & p) const
  {
    checkTwoDVectorParamsNoRespectiveOverlap<std::string>(p);
  }
  void wrapTwoDInnerVsOneD(const std::string & p1, const std::string & p2) const
  {
    checkTwoDVectorParamInnerSameLengthAsOneDVector<Real, Real>(p1, p2);
  }
  void wrapTwoDMMELength(const std::string & p1, const std::string & p2, const bool e2) const
  {
    checkTwoDVectorParamMultiMooseEnumSameLength<Real>(p1, p2, e2);
  }
  void wrapVecNotEmpty(const std::string & p) const { checkVectorParamNotEmpty<Real>(p); }
  void wrapVecSameLengthIfSet(const std::string & p1,
                              const std::string & p2,
                              const bool ignore = false) const
  {
    checkVectorParamsSameLengthIfSet<Real, Real>(p1, p2, ignore);
  }
  void wrapVecCombined(const std::string & p1, const std::string & p2, const std::string & p3) const
  {
    checkVectorParamLengthSameAsCombinedOthers<Real, Real, Real>(p1, p2, p3);
  }
  void wrapBlockwise(const std::string & bp, const std::vector<std::string> & pn) const
  {
    checkBlockwiseConsistency<Real>(bp, pn);
  }
  bool wrapParamConsistentReal(const InputParameters & other, const std::string & p) const
  {
    return parameterConsistent<Real>(other, p);
  }
  bool wrapParamConsistentEnum(const InputParameters & other, const std::string & p) const
  {
    return parameterConsistent<MooseEnum>(other, p);
  }
  void wrapWarnInconsistent(const InputParameters & other, const std::string & p) const
  {
    warnInconsistent<Real>(other, p);
  }
  void wrapErrorDependent(const std::string & p1,
                          const std::string & v,
                          const std::vector<std::string> & dp) const
  {
    errorDependentParameter(p1, v, dp);
  }
  void wrapErrorInconsistentDependent(const std::string & p1,
                                      const std::string & v,
                                      const std::vector<std::string> & dp) const
  {
    errorInconsistentDependentParameter(p1, v, dp);
  }
};

registerMooseObject("MooseUnitApp", CheckParamsTestObject);

InputParameters
CheckParamsTestObject::validParams()
{
  auto params = MooseObject::validParams();
  params.registerBase("CheckParamsTestObject");

  // Scalars
  params.addParam<bool>("flag", false, "Boolean parameter");
  params.addParam<Real>("A", "Optional scalar parameter A");
  params.addParam<Real>("B", "Optional scalar parameter B");
  params.addParam<Real>("C", "Optional scalar parameter C");

  // Enumerations
  params.addParam<MooseEnum>("menum", MooseEnum("x y z", "x"), "MooseEnum parameter");
  params.addParam<MultiMooseEnum>("mme", MultiMooseEnum("a b c d"), "MultiMooseEnum parameter");

  // Vectors
  params.addParam<std::vector<Real>>("vec_a", "Vector parameter a");
  params.addParam<std::vector<Real>>("vec_b", "Vector parameter b");
  params.addParam<std::vector<Real>>("vec_c", "Vector parameter c");
  // Valid-by-default empty vector, used to exercise the "ignore empty default" path
  params.addParam<std::vector<Real>>(
      "vec_empty_default", {}, "Vector parameter with empty default");

  // String vectors (the no-overlap checks operate on string-like items)
  params.addParam<std::vector<std::string>>("svec_a", "String vector parameter a");
  params.addParam<std::vector<std::string>>("svec_b", "String vector parameter b");

  // 2D vectors
  params.addParam<std::vector<std::vector<Real>>>("twod_a", "Two-D vector parameter a");
  params.addParam<std::vector<std::vector<Real>>>("twod_b", "Two-D vector parameter b");
  params.addParam<std::vector<std::vector<std::string>>>("stwod_a",
                                                         "Two-D string vector parameter a");

  // Groups of blocks for the block-wise consistency check
  params.addParam<std::vector<std::vector<SubdomainName>>>("block_groups", "Groups of blocks");

  return params;
}

/// Sets Moose::_warnings_are_errors for the duration of a scope so that warnings
/// become catchable exceptions, then restores it.
namespace
{
struct WarningsAsErrorsGuard
{
  WarningsAsErrorsGuard() { Moose::_warnings_are_errors = true; }
  ~WarningsAsErrorsGuard() { Moose::_warnings_are_errors = false; }
};
}

class InputParametersChecksUtilsTest : public MooseObjectUnitTest
{
public:
  InputParametersChecksUtilsTest() : MooseObjectUnitTest("MooseUnitApp") {}

protected:
  /// Fresh set of valid parameters for the test object
  InputParameters getParams() { return _factory.getValidParams("CheckParamsTestObject"); }

  /// Build a test object from a populated parameter set
  std::shared_ptr<CheckParamsTestObject> create(InputParameters & params)
  {
    params.set<SubProblem *>("_subproblem") = _fe_problem.get();
    return _factory.create<CheckParamsTestObject>(
        "CheckParamsTestObject", "test_object_" + std::to_string(_count++), params, 0);
  }

  /// Convenience: build a test object in which every Real parameter in \p set_params
  /// has been set by the user.
  std::shared_ptr<CheckParamsTestObject> buildObject(const std::vector<std::string> & set_params)
  {
    auto params = getParams();
    for (const auto & param : set_params)
      params.set<Real>(param) = 1.0;
    return create(params);
  }

  unsigned int _count = 0;
};

// ---------------------------------------------------------------------------
// checkAtMostOneParamSetByUser
// ---------------------------------------------------------------------------

// No parameter set by the user: the check passes.
TEST_F(InputParametersChecksUtilsTest, atMostOneNoneSet)
{
  auto obj = buildObject({});
  EXPECT_NO_THROW(obj->wrapAtMostOne({"A", "B", "C"}));
}

// Exactly one parameter set by the user: the check passes.
TEST_F(InputParametersChecksUtilsTest, atMostOneOneSet)
{
  auto obj = buildObject({"B"});
  EXPECT_NO_THROW(obj->wrapAtMostOne({"A", "B", "C"}));
}

// Two of the checked parameters set by the user: the check errors and reports
// the offending parameters.
TEST_F(InputParametersChecksUtilsTest, atMostOneTwoSet)
{
  auto obj = buildObject({"A", "C"});
  try
  {
    obj->wrapAtMostOne({"A", "B", "C"});
    FAIL() << "expected checkAtMostOneParamSetByUser to error";
  }
  catch (const std::exception & e)
  {
    const std::string msg(e.what());
    EXPECT_NE(msg.find("Only one parameter of"), std::string::npos) << msg;
    EXPECT_NE(msg.find("should be set but"), std::string::npos) << msg;
    EXPECT_NE(msg.find("A"), std::string::npos) << msg;
    EXPECT_NE(msg.find("C"), std::string::npos) << msg;
  }
}

// All checked parameters set by the user: the check errors.
TEST_F(InputParametersChecksUtilsTest, atMostOneAllSet)
{
  auto obj = buildObject({"A", "B", "C"});
  EXPECT_THROW(obj->wrapAtMostOne({"A", "B", "C"}), std::exception);
}

// Only parameters in the provided list are considered.
TEST_F(InputParametersChecksUtilsTest, atMostOneOnlyCountsListedParams)
{
  auto obj = buildObject({"A", "C"});
  EXPECT_NO_THROW(obj->wrapAtMostOne({"A", "B"}));
}

// ---------------------------------------------------------------------------
// checkParamsBothSetOrNotSet
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, bothSetOrNotSet)
{
  // Both set: OK
  EXPECT_NO_THROW(buildObject({"A", "B"})->wrapBothSetOrNotSet("A", "B"));
  // Neither set: OK
  EXPECT_NO_THROW(buildObject({})->wrapBothSetOrNotSet("A", "B"));
  // Only one set: error
  EXPECT_THROW(buildObject({"A"})->wrapBothSetOrNotSet("A", "B"), std::exception);
}

// ---------------------------------------------------------------------------
// checkSecondParamSetOnlyIfFirstOneTrue
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, secondParamSetOnlyIfFirstOneTrue)
{
  // flag false (default) and second set by user: error
  {
    auto params = getParams();
    params.set<Real>("A") = 1.0;
    EXPECT_THROW(create(params)->wrapSecondOnlyIfTrue("flag", "A"), std::exception);
  }
  // flag true and second set by user: OK
  {
    auto params = getParams();
    params.set<bool>("flag") = true;
    params.set<Real>("A") = 1.0;
    EXPECT_NO_THROW(create(params)->wrapSecondOnlyIfTrue("flag", "A"));
  }
  // flag false and second not set: OK
  EXPECT_NO_THROW(buildObject({})->wrapSecondOnlyIfTrue("flag", "A"));
}

// ---------------------------------------------------------------------------
// checkSecondParamSetOnlyIfFirstOneSet
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, secondParamSetOnlyIfFirstOneSet)
{
  // First not set, second set: error
  EXPECT_THROW(buildObject({"B"})->wrapSecondOnlyIfSet("A", "B"), std::exception);
  // Both set: OK
  EXPECT_NO_THROW(buildObject({"A", "B"})->wrapSecondOnlyIfSet("A", "B"));
  // First set, second not set: OK
  EXPECT_NO_THROW(buildObject({"A"})->wrapSecondOnlyIfSet("A", "B"));
}

// ---------------------------------------------------------------------------
// checkSecondParamNotSetIfFirstOneSet
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, secondParamNotSetIfFirstOneSet)
{
  // Both set: error
  EXPECT_THROW(buildObject({"A", "B"})->wrapSecondNotIfSet("A", "B"), std::exception);
  // Only first set: OK
  EXPECT_NO_THROW(buildObject({"A"})->wrapSecondNotIfSet("A", "B"));
  // Only second set: OK
  EXPECT_NO_THROW(buildObject({"B"})->wrapSecondNotIfSet("A", "B"));
}

// ---------------------------------------------------------------------------
// checkVectorParamsSameLength
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, vectorParamsSameLength)
{
  // Same length: OK
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    params.set<std::vector<Real>>("vec_b") = {3, 4};
    EXPECT_NO_THROW(create(params)->wrapVecSameLength("vec_a", "vec_b"));
  }
  // Different length (both set): error
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    params.set<std::vector<Real>>("vec_b") = {3};
    EXPECT_THROW(create(params)->wrapVecSameLength("vec_a", "vec_b"), std::exception);
  }
  // Only one set (with content): error via both-set-or-not-set
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    EXPECT_THROW(create(params)->wrapVecSameLength("vec_a", "vec_b"), std::exception);
  }
  // Neither set: OK
  EXPECT_NO_THROW(buildObject({})->wrapVecSameLength("vec_a", "vec_b"));
}

// ---------------------------------------------------------------------------
// checkVectorParamAndMultiMooseEnumLength
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, vectorParamAndMultiMooseEnumLength)
{
  // Same length: OK
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    params.set<MultiMooseEnum>("mme") = "a b";
    EXPECT_NO_THROW(create(params)->wrapVecMMELength("vec_a", "mme"));
  }
  // Different length: error
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    params.set<MultiMooseEnum>("mme") = "a";
    EXPECT_THROW(create(params)->wrapVecMMELength("vec_a", "mme"), std::exception);
  }
}

// ---------------------------------------------------------------------------
// checkTwoDVectorParamsSameLength
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, twoDVectorParamsSameLength)
{
  // Same outer and inner sizes: OK
  {
    auto params = getParams();
    params.set<std::vector<std::vector<Real>>>("twod_a") = {{1, 2}, {3, 4}};
    params.set<std::vector<std::vector<Real>>>("twod_b") = {{5, 6}, {7, 8}};
    EXPECT_NO_THROW(create(params)->wrapTwoDSameLength("twod_a", "twod_b"));
  }
  // Different outer size: error
  {
    auto params = getParams();
    params.set<std::vector<std::vector<Real>>>("twod_a") = {{1, 2}, {3, 4}};
    params.set<std::vector<std::vector<Real>>>("twod_b") = {{5, 6}};
    EXPECT_THROW(create(params)->wrapTwoDSameLength("twod_a", "twod_b"), std::exception);
  }
  // Same outer size but different inner size: error
  {
    auto params = getParams();
    params.set<std::vector<std::vector<Real>>>("twod_a") = {{1, 2}, {3, 4}};
    params.set<std::vector<std::vector<Real>>>("twod_b") = {{5, 6}, {7}};
    EXPECT_THROW(create(params)->wrapTwoDSameLength("twod_a", "twod_b"), std::exception);
  }
}

// ---------------------------------------------------------------------------
// checkVectorParamsNoOverlap
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, vectorParamsNoOverlap)
{
  // Disjoint vectors: OK
  {
    auto params = getParams();
    params.set<std::vector<std::string>>("svec_a") = {"x", "y"};
    params.set<std::vector<std::string>>("svec_b") = {"z", "w"};
    EXPECT_NO_THROW(create(params)->wrapVecNoOverlap({"svec_a", "svec_b"}));
  }
  // Overlap across two vectors: error
  {
    auto params = getParams();
    params.set<std::vector<std::string>>("svec_a") = {"x", "y"};
    params.set<std::vector<std::string>>("svec_b") = {"y", "z"};
    EXPECT_THROW(create(params)->wrapVecNoOverlap({"svec_a", "svec_b"}), std::exception);
  }
  // Repeated item within a single vector: error
  {
    auto params = getParams();
    params.set<std::vector<std::string>>("svec_a") = {"x", "x"};
    EXPECT_THROW(create(params)->wrapVecNoOverlap({"svec_a"}), std::exception);
  }
}

// ---------------------------------------------------------------------------
// checkTwoDVectorParamsNoRespectiveOverlap
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, twoDVectorParamsNoRespectiveOverlap)
{
  // No repeats within any inner vector: OK
  {
    auto params = getParams();
    params.set<std::vector<std::vector<std::string>>>("stwod_a") = {{"x", "y"}, {"z", "w"}};
    EXPECT_NO_THROW(create(params)->wrapTwoDNoOverlap({"stwod_a"}));
  }
  // Repeated item within an inner vector: error
  {
    auto params = getParams();
    params.set<std::vector<std::vector<std::string>>>("stwod_a") = {{"x", "x"}};
    EXPECT_THROW(create(params)->wrapTwoDNoOverlap({"stwod_a"}), std::exception);
  }
}

// ---------------------------------------------------------------------------
// checkTwoDVectorParamInnerSameLengthAsOneDVector
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, twoDVectorParamInnerSameLengthAsOneDVector)
{
  // Every inner vector matches the 1D vector length: OK
  {
    auto params = getParams();
    params.set<std::vector<std::vector<Real>>>("twod_a") = {{1, 2}, {3, 4}};
    params.set<std::vector<Real>>("vec_a") = {5, 6};
    EXPECT_NO_THROW(create(params)->wrapTwoDInnerVsOneD("twod_a", "vec_a"));
  }
  // An inner vector differs in length: error
  {
    auto params = getParams();
    params.set<std::vector<std::vector<Real>>>("twod_a") = {{1, 2}, {3, 4}};
    params.set<std::vector<Real>>("vec_a") = {5, 6, 7};
    EXPECT_THROW(create(params)->wrapTwoDInnerVsOneD("twod_a", "vec_a"), std::exception);
  }
}

// ---------------------------------------------------------------------------
// checkTwoDVectorParamMultiMooseEnumSameLength
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, twoDVectorParamMultiMooseEnumSameLength)
{
  // Unrolled 2D size (2x2) equals the enumeration size (4): OK
  {
    auto params = getParams();
    params.set<std::vector<std::vector<Real>>>("twod_a") = {{1, 2}, {3, 4}};
    params.set<MultiMooseEnum>("mme") = "a b c d";
    EXPECT_NO_THROW(create(params)->wrapTwoDMMELength("twod_a", "mme", false));
  }
  // Size mismatch, error attributed to the 2D vector parameter
  {
    auto params = getParams();
    params.set<std::vector<std::vector<Real>>>("twod_a") = {{1, 2}, {3, 4}};
    params.set<MultiMooseEnum>("mme") = "a b";
    EXPECT_THROW(create(params)->wrapTwoDMMELength("twod_a", "mme", false), std::exception);
  }
  // Size mismatch, error attributed to the enumeration parameter
  {
    auto params = getParams();
    params.set<std::vector<std::vector<Real>>>("twod_a") = {{1, 2}, {3, 4}};
    params.set<MultiMooseEnum>("mme") = "a b";
    EXPECT_THROW(create(params)->wrapTwoDMMELength("twod_a", "mme", true), std::exception);
  }
}

// ---------------------------------------------------------------------------
// checkVectorParamNotEmpty
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, vectorParamNotEmpty)
{
  // Non-empty: OK
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1};
    EXPECT_NO_THROW(create(params)->wrapVecNotEmpty("vec_a"));
  }
  // Empty: error
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {};
    EXPECT_THROW(create(params)->wrapVecNotEmpty("vec_a"), std::exception);
  }
}

// ---------------------------------------------------------------------------
// checkVectorParamsSameLengthIfSet
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, vectorParamsSameLengthIfSet)
{
  // Both set, same length: OK
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    params.set<std::vector<Real>>("vec_b") = {3, 4};
    EXPECT_NO_THROW(create(params)->wrapVecSameLengthIfSet("vec_a", "vec_b"));
  }
  // Both set, different length: error
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    params.set<std::vector<Real>>("vec_b") = {3};
    EXPECT_THROW(create(params)->wrapVecSameLengthIfSet("vec_a", "vec_b"), std::exception);
  }
  // Only one set: OK (the other is not valid, so nothing is compared)
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    EXPECT_NO_THROW(create(params)->wrapVecSameLengthIfSet("vec_a", "vec_b"));
  }
  // Second is an empty default not set by the user, ignore flag true: OK
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    EXPECT_NO_THROW(create(params)->wrapVecSameLengthIfSet("vec_a", "vec_empty_default", true));
  }
  // Same case with ignore flag false: error (sizes 2 and 0 differ)
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    EXPECT_THROW(create(params)->wrapVecSameLengthIfSet("vec_a", "vec_empty_default", false),
                 std::exception);
  }
}

// ---------------------------------------------------------------------------
// checkVectorParamLengthSameAsCombinedOthers
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, vectorParamLengthSameAsCombinedOthers)
{
  // size(vec_a) == size(vec_b) + size(vec_c): OK
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2, 3};
    params.set<std::vector<Real>>("vec_b") = {4};
    params.set<std::vector<Real>>("vec_c") = {5, 6};
    EXPECT_NO_THROW(create(params)->wrapVecCombined("vec_a", "vec_b", "vec_c"));
  }
  // Combined size does not match: error
  {
    auto params = getParams();
    params.set<std::vector<Real>>("vec_a") = {1, 2, 3};
    params.set<std::vector<Real>>("vec_b") = {4};
    params.set<std::vector<Real>>("vec_c") = {5};
    EXPECT_THROW(create(params)->wrapVecCombined("vec_a", "vec_b", "vec_c"), std::exception);
  }
}

// ---------------------------------------------------------------------------
// checkBlockwiseConsistency
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, blockwiseConsistencyWithBlocks)
{
  // Two block groups, one entry per group in the checked parameter: OK
  {
    auto params = getParams();
    params.set<std::vector<std::vector<SubdomainName>>>("block_groups") = {{"1"}, {"2"}};
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    auto obj = create(params);
    obj->_blocks = {"1", "2"};
    EXPECT_NO_THROW(obj->wrapBlockwise("block_groups", {"vec_a"}));
  }
  // Number of parameter entries does not match the number of block groups: error
  {
    auto params = getParams();
    params.set<std::vector<std::vector<SubdomainName>>>("block_groups") = {{"1"}, {"2"}};
    params.set<std::vector<Real>>("vec_a") = {1};
    auto obj = create(params);
    obj->_blocks = {"1", "2"};
    EXPECT_THROW(obj->wrapBlockwise("block_groups", {"vec_a"}), std::exception);
  }
  // A referenced block is not part of the object's block restriction: error
  {
    auto params = getParams();
    params.set<std::vector<std::vector<SubdomainName>>>("block_groups") = {{"3"}};
    params.set<std::vector<Real>>("vec_a") = {1};
    auto obj = create(params);
    obj->_blocks = {"1", "2"};
    EXPECT_THROW(obj->wrapBlockwise("block_groups", {"vec_a"}), std::exception);
  }
}

TEST_F(InputParametersChecksUtilsTest, blockwiseConsistencyWithoutBlocks)
{
  // No block groups: parameters may have one entry each and must all match: OK
  {
    auto params = getParams();
    params.set<std::vector<std::vector<SubdomainName>>>("block_groups") = {};
    params.set<std::vector<Real>>("vec_a") = {1};
    params.set<std::vector<Real>>("vec_b") = {2};
    EXPECT_NO_THROW(create(params)->wrapBlockwise("block_groups", {"vec_a", "vec_b"}));
  }
  // No block groups but the first parameter has more than one entry: error
  {
    auto params = getParams();
    params.set<std::vector<std::vector<SubdomainName>>>("block_groups") = {};
    params.set<std::vector<Real>>("vec_a") = {1, 2};
    EXPECT_THROW(create(params)->wrapBlockwise("block_groups", {"vec_a"}), std::exception);
  }
  // No block groups and the parameters have inconsistent sizes: error
  {
    auto params = getParams();
    params.set<std::vector<std::vector<SubdomainName>>>("block_groups") = {};
    params.set<std::vector<Real>>("vec_a") = {1};
    params.set<std::vector<Real>>("vec_b") = {};
    EXPECT_THROW(create(params)->wrapBlockwise("block_groups", {"vec_a", "vec_b"}), std::exception);
  }
}

// ---------------------------------------------------------------------------
// parameterConsistent
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, parameterConsistent)
{
  auto make = [this](Real a)
  {
    auto params = getParams();
    params.set<Real>("A") = a;
    return create(params);
  };

  // Same value: consistent
  {
    auto obj = make(1.0);
    auto other = make(1.0);
    EXPECT_TRUE(obj->wrapParamConsistentReal(other->parameters(), "A"));
  }
  // Different value: inconsistent
  {
    auto obj = make(1.0);
    auto other = make(2.0);
    EXPECT_FALSE(obj->wrapParamConsistentReal(other->parameters(), "A"));
  }
  // Other object does not set the parameter: treated as consistent
  {
    auto obj = make(1.0);
    auto other = buildObject({});
    EXPECT_TRUE(obj->wrapParamConsistentReal(other->parameters(), "A"));
  }
  // MooseEnum branch: same selection is consistent
  {
    auto obj = buildObject({});
    auto other = buildObject({});
    EXPECT_TRUE(obj->wrapParamConsistentEnum(other->parameters(), "menum"));
  }
  // MooseEnum branch: different selection is inconsistent
  {
    auto obj = buildObject({});
    auto params = getParams();
    params.set<MooseEnum>("menum") = "y";
    auto other = create(params);
    EXPECT_FALSE(obj->wrapParamConsistentEnum(other->parameters(), "menum"));
  }
}

// ---------------------------------------------------------------------------
// warnInconsistent
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, warnInconsistent)
{
  WarningsAsErrorsGuard guard;

  // Inconsistent parameters: a warning is emitted (thrown here)
  {
    auto params = getParams();
    params.set<Real>("A") = 1.0;
    auto obj = create(params);
    auto other_params = getParams();
    other_params.set<Real>("A") = 2.0;
    auto other = create(other_params);
    try
    {
      obj->wrapWarnInconsistent(other->parameters(), "A");
      ADD_FAILURE() << "expected warnInconsistent to emit a warning";
    }
    catch (const std::exception & e)
    {
      EXPECT_NE(std::string(e.what()).find("is inconsistent between"), std::string::npos)
          << e.what();
    }
  }
  // Consistent parameters: no warning
  {
    auto params = getParams();
    params.set<Real>("A") = 1.0;
    auto obj = create(params);
    auto other_params = getParams();
    other_params.set<Real>("A") = 1.0;
    auto other = create(other_params);
    EXPECT_NO_THROW(obj->wrapWarnInconsistent(other->parameters(), "A"));
  }
}

// ---------------------------------------------------------------------------
// errorDependentParameter
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, errorDependentParameter)
{
  // A dependent parameter is set by the user: error
  EXPECT_THROW(buildObject({"B"})->wrapErrorDependent("A", "true", {"B", "C"}), std::exception);
  // No dependent parameter set: OK
  EXPECT_NO_THROW(buildObject({})->wrapErrorDependent("A", "true", {"B", "C"}));
}

// ---------------------------------------------------------------------------
// errorInconsistentDependentParameter
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, errorInconsistentDependentParameter)
{
  // A dependent parameter is set by the user: error
  EXPECT_THROW(buildObject({"C"})->wrapErrorInconsistentDependent("A", "false", {"B", "C"}),
               std::exception);
  // No dependent parameter set: OK
  EXPECT_NO_THROW(buildObject({})->wrapErrorInconsistentDependent("A", "false", {"B", "C"}));
}

// ---------------------------------------------------------------------------
// assertParamDefined (debug-only assertion; here we exercise the passing path)
// ---------------------------------------------------------------------------

TEST_F(InputParametersChecksUtilsTest, assertParamDefined)
{
  // A defined parameter does not trip the assertion. The failure path is a
  // mooseAssert, which is only active in a debug build.
  EXPECT_NO_THROW(buildObject({})->wrapAssertParamDefined<Real>("A"));
}
