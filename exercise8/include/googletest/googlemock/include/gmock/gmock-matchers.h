// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// PPI, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Google Mock - a framework for writing C++ mock classes.
//
// The MATCHER* family of macros can be used in a namespace scope to
// define custom matchers easily.
//
// Basic Usage
// ===========
//
// The syntax
//
//   MATCHER(name, description_string) { statements; }
//
// defines a matcher with the given name that executes the statements,
// which must return a bool to indicate if the match succeeds.  Inside
// the statements, you can refer to the value being matched by 'arg',
// and refer to its type by 'arg_type'.
//
// The description string documents what the matcher does, and is used
// to generate the failure message when the match fails.  Since a
// MATCHER() is usually defined in a header file shared by multiple
// C++ source files, we require the description to be a C-string
// literal to avoid possible side effects.  It can be empty, in which
// case we'll use the sequence of words in the matcher name as the
// description.
//
// For example:
//
//   MATCHER(IsEven, "") { return (arg % 2) == 0; }
//
// allows you to write
//
//   // Expects mock_foo.Bar(n) to be called where n is even.
//   EXPECT_CALL(mock_foo, Bar(IsEven()));
//
// or,
//
//   // Verifies that the value of some_expression is even.
//   EXPECT_THAT(some_expression, IsEven());
//
// If the above assertion fails, it will print something like:
//
//   Value of: some_expression
//   Expected: is even
//     Actual: 7
//
// where the description "is even" is automatically calculated from the
// matcher name IsEven.
//
// Argument Type
// =============
//
// Note that the type of the value being matched (arg_type) is
// determined by the context in which you use the matcher and is
// supplied to you by the compiler, so you don't need to worry about
// declaring it (nor can you).  This allows the matcher to be
// polymorphic.  For example, IsEven() can be used to match any type
// where the value of "(arg % 2) == 0" can be implicitly converted to
// a bool.  In the "Bar(IsEven())" example above, if method Bar()
// takes an int, 'arg_type' will be int; if it takes an unsigned long,
// 'arg_type' will be unsigned long; and so on.
//
// Parameterizing Matchers
// =======================
//
// Sometimes you'll want to parameterize the matcher.  For that you
// can use another macro:
//
//   MATCHER_P(name, param_name, description_string) { statements; }
//
// For example:
//
//   MATCHER_P(HasAbsoluteValue, value, "") { return abs(arg) == value; }
//
// will allow you to write:
//
//   EXPECT_THAT(Blah("a"), HasAbsoluteValue(n));
//
// which may lead to this message (assuming n is 10):
//
//   Value of: Blah("a")
//   Expected: has absolute value 10
//     Actual: -9
//
// Note that both the matcher description and its parameter are
// printed, making the message human-friendly.
//
// In the matcher definition body, you can write 'foo_type' to
// reference the type of a parameter named 'foo'.  For example, in the
// body of MATCHER_P(HasAbsoluteValue, value) above, you can write
// 'value_type' to refer to the type of 'value'.
//
// We also provide MATCHER_P2, MATCHER_P3, ..., up to MATCHER_P$n to
// support multi-parameter matchers.
//
// Describing Parameterized Matchers
// =================================
//
// The last argument to MATCHER*() is a string-typed expression.  The
// expression can reference all of the matcher's parameters and a
// special bool-typed variable named 'negation'.  When 'negation' is
// false, the expression should evaluate to the matcher's description;
// otherwise it should evaluate to the description of the negation of
// the matcher.  For example,
//
//   using testing::PrintToString;
//
//   MATCHER_P2(InClosedRange, low, hi,
//       std::string(negation ? "is not" : "is") + " in range [" +
//       PrintToString(low) + ", " + PrintToString(hi) + "]") {
//     return low <= arg && arg <= hi;
//   }
//   ...
//   EXPECT_THAT(3, InClosedRange(4, 6));
//   EXPECT_THAT(3, Not(InClosedRange(2, 4)));
//
// would generate two failures that contain the text:
//
//   Expected: is in range [4, 6]
//   ...
//   Expected: is not in range [2, 4]
//
// If you specify "" as the description, the failure message will
// contain the sequence of words in the matcher name followed by the
// parameter values printed as a tuple.  For example,
//
//   MATCHER_P2(InClosedRange, low, hi, "") { ... }
//   ...
//   EXPECT_THAT(3, InClosedRange(4, 6));
//   EXPECT_THAT(3, Not(InClosedRange(2, 4)));
//
// would generate two failures that contain the text:
//
//   Expected: in closed range (4, 6)
//   ...
//   Expected: not (in closed range (2, 4))
//
// Types of Matcher Parameters
// ===========================
//
// For the purpose of typing, you can view
//
//   MATCHER_Pk(Foo, p1, ..., pk, description_string) { ... }
//
// as shorthand for
//
//   template <typename p1_type, ..., typename pk_type>
//   FooMatcherPk<p1_type, ..., pk_type>
//   Foo(p1_type p1, ..., pk_type pk) { ... }
//
// When you write Foo(v1, ..., vk), the compiler infers the types of
// the parameters v1, ..., and vk for you.  If you are not happy with
// the result of the type inference, you can specify the types by
// explicitly instantiating the template, as in Foo<long, bool>(5,
// false).  As said earlier, you don't get to (or need to) specify
// 'arg_type' as that's determined by the context in which the matcher
// is used.  You can assign the result of expression Foo(p1, ..., pk)
// to a variable of type FooMatcherPk<p1_type, ..., pk_type>.  This
// can be useful when composing matchers.
//
// While you can instantiate a matcher template with reference types,
// passing the parameters by pointer usually makes your code more
// readable.  If, however, you still want to pass a parameter by
// reference, be aware that in the failure message generated by the
// matcher you will see the value of the referenced object but not its
// address.
//
// Explaining Match Results
// ========================
//
// Sometimes the matcher description alone isn't enough to explain why
// the match has failed or succeeded.  For example, when expecting a
// long string, it can be very helpful to also print the diff between
// the expected string and the actual one.  To achieve that, you can
// optionally stream additional information to a special variable
// named result_listener, whose type is a pointer to class
// MatchResultListener:
//
//   MATCHER_P(EqualsLongString, str, "") {
//     if (arg == str) return true;
//
//     *result_listener << "the difference: "
///                     << DiffStrings(str, arg);
//     return false;
//   }
//
// Overloading Matchers
// ====================
//
// You can overload matchers with different numbers of parameters:
//
//   MATCHER_P(Blah, a, description_string1) { ... }
//   MATCHER_P2(Blah, a, b, description_string2) { ... }
//
// Caveats
// =======
//
// When defining a new matcher, you should also consider implementing
// MatcherInterface or using MakePolymorphicMatcher().  These
// approaches require more work than the MATCHER* macros, but also
// give you more control on the types of the value being matched and
// the matcher parameters, which may leads to better compiler error
// messages when the matcher is used wrong.  They also allow
// overloading matchers based on parameter types (as opposed to just
// based on the number of parameters).
//
// MATCHER*() can only be used in a namespace scope as templates cannot be
// declared inside of a local class.
//
// More Information
// ================
//
// To learn more about using these macros, please search for 'MATCHER'
// on
// https://github.com/google/googletest/blob/main/docs/gmock_cook_book.md
//
// This file also implements some commonly used argument matchers.  More
// matchers can be defined by the user implementing the
// MatcherInterface<T> interface if necessary.
//
// See googletest/include/gtest/gtest-matchers.h for the definition of class
// Matcher, class MatcherInterface, and others.

// IWYU pragma: private, include "gmock/gmock.h"
// IWYU pragma: friend gmock/.*

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_

#include <algorithm>
#include <cmath>
#include <exception>
#include <functional>
#include <initializer_list>
#include <ios>
#include <iterator>
#include <limits>
#include <memory>
#include <ostream>  // NOLINT
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/internal/gmock-internal-utils.h"
#include "gmock/internal/gmock-port.h"
#include "gmock/internal/gmock-pp.h"
#include "gtest/gtest.h"

// MSVC warning C5046 is new as of VS2017 version 15.8.
#if defined(_MSC_VER) && _MSC_VER >= 1915
#define GMOCK_MAYBE_5046_ 5046
#else
#define GMOCK_MAYBE_5046_
#endif

GTEST_DISABLE_MSC_WARNINGS_PUSH_(
    4251 GMOCK_MAYBE_5046_ /* class A needs to have dll-interface to be used by
                              clients of class B */
    /* Symbol involving type with internal linkage not defined */)

namespace testing {

// To implement a matcher Foo for type T, define:
//   1. a class FooMatcherImpl that implements the
//      MatcherInterface<T> interface, and
//   2. a factory function that creates a Matcher<T> object from a
//      FooMatcherImpl*.
//
// The two-level delegation design makes it possible to allow a user
// to write "v" instead of "Eq(v)" where a Matcher is expected, which
// is impossible if we pass matchers by pointers.  It also eases
// ownership management as Matcher objects can now be copied like
// plain values.

// A match result listener that stores the explanation in a string.
class StringMatchResultListener : public MatchResultListener {
 public:
  StringMatchResultListener() : MatchResultListener(&ss_) {}

  // Returns the explanation accumulated so far.
  std::string str() const { return ss_.str(); }

  // Clears the explanation accumulated so far.
  void Clear() { ss_.str(""); }

 private:
  ::std::stringstream ss_;

  StringMatchResultListener(const StringMatchResultListener&) = delete;
  StringMatchResultListener& operator=(const StringMatchResultListener&) =
      delete;
};

// Anything inside the 'internal' namespace IS INTERNAL IMPLEMENTATION
// and MUST NOT BE USED IN USER CODE!!!
namespace internal {

// The MatcherCastImpl class template is a helper for implementing
// MatcherCast().  We need this helper in order to partially
// specialize the implementation of MatcherCast() (C++ allows
// class/struct templates to be partially specialized, but not
// function templates.).

// This general version is used when MatcherCast()'s argument is a
// polymorphic matcher (i.e. something that can be converted to a
// Matcher but is not one yet; for example, Eq(value)) or a value (for
// example, "hello").
template <typename T, typename M>
class MatcherCastImpl {
 public:
  static Matcher<T> Cast(const M& polymorphic_matcher_or_value) {
    // M can be a polymorphic matcher, in which case we want to use
    // its conversion operator to create Matcher<T>.  Or it can be a value
    // that should be passed to the Matcher<T>'s constructor.
    //
    // We can't call Matcher<T>(polymorphic_matcher_or_value) when M is a
    // polymorphic matcher because it'll be ambiguous if T has an implicit
    // constructor from M (this usually happens when T has an implicit
    // constructor from any type).
    //
    // It won't work to unconditionally implicit_cast
    // polymorphic_matcher_or_value to Matcher<T> because it won't trigger
    // a user-defined conversion from M to T if one exists (assuming M is
    // a value).
    return CastImpl(polymorphic_matcher_or_value,
                    std::is_convertible<M, Matcher<T>>{},
                    std::is_convertible<M, T>{});
  }

 private:
  template <bool Ignore>
  static Matcher<T> CastImpl(const M& polymorphic_matcher_or_value,
                             std::true_type /* convertible_to_matcher */,
                             std::integral_constant<bool, Ignore>) {
    // M is implicitly convertible to Matcher<T>, which means that either
    // M is a polymorphic matcher or Matcher<T> has an implicit constructor
    // from M.  In both cases using the implicit conversion will produce a
    // matcher.
    //
    // Even if T has an implicit constructor from M, it won't be called because
    // creating Matcher<T> would require a chain of two user-defined conversions
    // (first to create T from M and then to create Matcher<T> from T).
    return polymorphic_matcher_or_value;
  }

  // M can't be implicitly converted to Matcher<T>, so M isn't a polymorphic
  // matcher. It's a value of a type implicitly convertible to T. Use direct
  // initialization to create a matcher.
  static Matcher<T> CastImpl(const M& value,
                             std::false_type /* convertible_to_matcher */,
                             std::true_type /* convertible_to_T */) {
    return Matcher<T>(ImplicitCast_<T>(value));
  }

  // M can't be implicitly converted to either Matcher<T> or T. Attempt to use
  // polymorphic matcher Eq(value) in this case.
  //
  // Note that we first attempt to perform an implicit cast on the value and
  // only fall back to the polymorphic Eq() matcher afterwards because the
  // latter calls bool operator==(const Lhs& lhs, const Rhs& rhs) in the end
  // which might be undefined even when Rhs is implicitly convertible to Lhs
  // (e.g. std::pair<const int, int> vs. std::pair<int, int>).
  //
  // We don't define this method inline as we need the declaration of Eq().
  static Matcher<T> CastImpl(const M& value,
                             std::false_type /* convertible_to_matcher */,
                             std::false_type /* convertible_to_T */);
};

// This more specialized version is used when MatcherCast()'s argument
// is already a Matcher.  This only compiles when type T can be
// statically converted to type U.
template <typename T, typename U>
class MatcherCastImpl<T, Matcher<U>> {
 public:
  static Matcher<T> Cast(const Matcher<U>& source_matcher) {
    return Matcher<T>(new Impl(source_matcher));
  }

 private:
  class Impl : public MatcherInterface<T> {
   public:
    explicit Impl(const Matcher<U>& source_matcher)
        : source_matcher_(source_matcher) {}

    // We delegate the matching logic to the source matcher.
    bool MatchAndExplain(T x, MatchResultListener* listener) const override {
      using FromType = typename std::remove_cv<typename std::remove_pointer<
          typename std::remove_reference<T>::type>::type>::type;
      using ToType = typename std::remove_cv<typename std::remove_pointer<
          typename std::remove_reference<U>::type>::type>::type;
      // Do not allow implicitly converting base*/& to derived*/&.
      static_assert(
          // Do not trigger if only one of them is a pointer. That implies a
          // regular conversion and not a down_cast.
          (std::is_pointer<typename std::remove_reference<T>::type>::value !=
           std::is_pointer<typename std::remove_reference<U>::type>::value) ||
              std::is_same<FromType, ToType>::value ||
              !std::is_base_of<FromType, ToType>::value,
          "Can't implicitly convert from <base> to <derived>");

      // Do the cast to `U` explicitly if necessary.
      // Otherwise, let implicit conversions do the trick.
      using CastType =
          typename std::conditional<std::is_convertible<T&, const U&>::value,
                                    T&, U>::type;

      return source_matcher_.MatchAndExplain(static_cast<CastType>(x),
                                             listener);
    }

    void DescribeTo(::std::ostream* os) const override {
      source_matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      source_matcher_.DescribeNegationTo(os);
    }

   private:
    const Matcher<U> source_matcher_;
  };
};

// This even more specialized version is used for efficiently casting
// a matcher to its own type.
template <typename T>
class MatcherCastImpl<T, Matcher<T>> {
 public:
  static Matcher<T> Cast(const Matcher<T>& matcher) { return matcher; }
};

// Template specialization for parameterless Matcher.
template <typename Derived>
class MatcherBaseImpl {
 public:
  MatcherBaseImpl() = default;

  template <typename T>
  operator ::testing::Matcher<T>() const {  // NOLINT(runtime/explicit)
    return ::testing::Matcher<T>(new
                                 typename Derived::template gmock_Impl<T>());
  }
};

// Template specialization for Matcher with parameters.
template <template <typename...> class Derived, typename... Ts>
class MatcherBaseImpl<Derived<Ts...>> {
 public:
  // Mark the constructor explicit for single argument T to avoid implicit
  // conversions.
  template <typename E = std::enable_if<sizeof...(Ts) == 1>,
            typename E::type* = nullptr>
  explicit MatcherBaseImpl(Ts... params)
      : params_(std::forward<Ts>(params)...) {}
  template <typename E = std::enable_if<sizeof...(Ts) != 1>,
            typename = typename E::type>
  MatcherBaseImpl(Ts... params)  // NOLINT
      : params_(std::forward<Ts>(params)...) {}

  template <typename F>
  operator ::testing::Matcher<F>() const {  // NOLINT(runtime/explicit)
    return Apply<F>(std::make_index_sequence<sizeof...(Ts)>{});
  }

 private:
  template <typename F, std::size_t... tuple_ids>
  ::testing::Matcher<F> Apply(std::index_sequence<tuple_ids...>) const {
    return ::testing::Matcher<F>(
        new typename Derived<Ts...>::template gmock_Impl<F>(
            std::get<tuple_ids>(params_)...));
  }

  const std::tuple<Ts...> params_;
};

}  // namespace internal

// In order to be safe and clear, casting between different matcher
// types is done explicitly via MatcherCast<T>(m), which takes a
// matcher m and returns a Matcher<T>.  It compiles only when T can be
// statically converted to the argument type of m.
template <typename T, typename M>
inline Matcher<T> MatcherCast(const M& matcher) {
  return internal::MatcherCastImpl<T, M>::Cast(matcher);
}

// This overload handles polymorphic matchers and values only since
// monomorphic matchers are handled by the next one.
template <typename T, typename M>
inline Matcher<T> SafeMatcherCast(const M& polymorphic_matcher_or_value) {
  return MatcherCast<T>(polymorphic_matcher_or_value);
}

// This overload handles monomorphic matchers.
//
// In general, if type T can be implicitly converted to type U, we can
// safely convert a Matcher<U> to a Matcher<T> (i.e. Matcher is
// contravariant): just keep a copy of the original Matcher<U>, convert the
// argument from type T to U, and then pass it to the underlying Matcher<U>.
// The only exception is when U is a reference and T is not, as the
// underlying Matcher<U> may be interested in the argument's address, which
// is not preserved in the conversion from T to U.
template <typename T, typename U>
inline Matcher<T> SafeMatcherCast(const Matcher<U>& matcher) {
  // Enforce that T can be implicitly converted to U.
  static_assert(std::is_convertible<const T&, const U&>::value,
                "T must be implicitly convertible to U");
  // Enforce that we are not converting a non-reference type T to a reference
  // type U.
  static_assert(std::is_reference<T>::value || !std::is_reference<U>::value,
                "cannot convert non reference arg to reference");
  // In case both T and U are arithmetic types, enforce that the
  // conversion is not lossy.
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(T) RawT;
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(U) RawU;
  constexpr bool kTIsOther = GMOCK_KIND_OF_(RawT) == internal::kOther;
  constexpr bool kUIsOther = GMOCK_KIND_OF_(RawU) == internal::kOther;
  static_assert(
      kTIsOther || kUIsOther ||
          (internal::LosslessArithmeticConvertible<RawT, RawU>::value),
      "conversion of arithmetic types must be lossless");
  return MatcherCast<T>(matcher);
}

// A<T>() returns a matcher that matches any value of type T.
template <typename T>
Matcher<T> A();

// Anything inside the 'internal' namespace IS INTERNAL IMPLEMENTATION
// and MUST NOT BE USED IN USER CODE!!!
namespace internal {

// If the explanation is not empty, prints it to the ostream.
inline void PrintIfNotEmpty(const std::string& explanation,
                            ::std::ostream* os) {
  if (!explanation.empty() && os != nullptr) {
    *os << ", " << explanation;
  }
}

// Returns true if the given type name is easy to read by a human.
// This is used to decide whether printing the type of a value might
// be helpful.
inline bool IsReadableTypeName(const std::string& type_name) {
  // We consider a type name readable if it's short or doesn't contain
  // a template or function type.
  return (type_name.length() <= 20 ||
          type_name.find_first_of("<(") == std::string::npos);
}

// Matches the value against the given matcher, prints the value and explains
// the match result to the listener. Returns the match result.
// 'listener' must not be NULL.
// Value cannot be passed by const reference, because some matchers take a
// non-const argument.
template <typename Value, typename T>
bool MatchPrintAndExplain(Value& value, const Matcher<T>& matcher,
                          MatchResultListener* listener) {
  if (!listener->IsInterested()) {
    // If the listener is not interested, we do not need to construct the
    // inner explanation.
    return matcher.Matches(value);
  }

  StringMatchResultListener inner_listener;
  const bool match = matcher.MatchAndExplain(value, &inner_listener);

  UniversalPrint(value, listener->stream());
#if GTEST_HAS_RTTI
  const std::string& type_name = GetTypeName<Value>();
  if (IsReadableTypeName(type_name))
    *listener->stream() << " (of type " << type_name << ")";
#endif
  PrintIfNotEmpty(inner_listener.str(), listener->stream());

  return match;
}

// An internal helper class for doing compile-time loop on a tuple's
// fields.
template <size_t N>
class TuplePrefix {
 public:
  // TuplePrefix<N>::Matches(matcher_tuple, value_tuple) returns true
  // if and only if the first N fields of matcher_tuple matches
  // the first N fields of value_tuple, respectively.
  template <typename MatcherTuple, typename ValueTuple>
  static bool Matches(const MatcherTuple& matcher_tuple,
                      const ValueTuple& value_tuple) {
    return TuplePrefix<N - 1>::Matches(matcher_tuple, value_tuple) &&
           std::get<N - 1>(matcher_tuple).Matches(std::get<N - 1>(value_tuple));
  }

  // TuplePrefix<N>::ExplainMatchFailuresTo(matchers, values, os)
  // describes failures in matching the first N fields of matchers
  // against the first N fields of values.  If there is no failure,
  // nothing will be streamed to os.
  template <typename MatcherTuple, typename ValueTuple>
  static void ExplainMatchFailuresTo(const MatcherTuple& matchers,
                                     const ValueTuple& values,
                                     ::std::ostream* os) {
    // First, describes failures in the first N - 1 fields.
    TuplePrefix<N - 1>::ExplainMatchFailuresTo(matchers, values, os);

    // Then describes the failure (if any) in the (N - 1)-th (0-based)
    // field.
    typename std::tuple_element<N - 1, MatcherTuple>::type matcher =
        std::get<N - 1>(matchers);
    typedef typename std::tuple_element<N - 1, ValueTuple>::type Value;
    const Value& value = std::get<N - 1>(values);
    StringMatchResultListener listener;
    if (!matcher.MatchAndExplain(value, &listener)) {
      *os << "  Expected arg #" << N - 1 << ": ";
      std::get<N - 1>(matchers).DescribeTo(os);
      *os << "\n           Actual: ";
      // We remove the reference in type Value to prevent the
      // universal printer from printing the address of value, which
      // isn't interesting to the user most of the time.  The
      // matcher's MatchAndExplain() method handles the case when
      // the address is interesting.
      internal::UniversalPrint(value, os);
      PrintIfNotEmpty(listener.str(), os);
      *os << "\n";
    }
  }
};

// The base case.
template <>
class TuplePrefix<0> {
 public:
  template <typename MatcherTuple, typename ValueTuple>
  static bool Matches(const MatcherTuple& /* matcher_tuple */,
                      const ValueTuple& /* value_tuple */) {
    return true;
  }

  template <typename MatcherTuple, typename ValueTuple>
  static void ExplainMatchFailuresTo(const MatcherTuple& /* matchers */,
                                     const ValueTuple& /* values */,
                                     ::std::ostream* /* os */) {}
};

// TupleMatches(matcher_tuple, value_tuple) returns true if and only if
// all matchers in matcher_tuple match the corresponding fields in
// value_tuple.  It is a compiler error if matcher_tuple and
// value_tuple have different number of fields or incompatible field
// types.
template <typename MatcherTuple, typename ValueTuple>
bool TupleMatches(const MatcherTuple& matcher_tuple,
                  const ValueTuple& value_tuple) {
  // Makes sure that matcher_tuple and value_tuple have the same
  // number of fields.
  static_assert(std::tuple_size<MatcherTuple>::value ==
                    std::tuple_size<ValueTuple>::value,
                "matcher and value have different numbers of fields");
  return TuplePrefix<std::tuple_size<ValueTuple>::value>::Matches(matcher_tuple,
                                                                  value_tuple);
}

// Describes failures in matching matchers against values.  If there
// is no failure, nothing will be streamed to os.
template <typename MatcherTuple, typename ValueTuple>
void ExplainMatchFailureTupleTo(const MatcherTuple& matchers,
                                const ValueTuple& values, ::std::ostream* os) {
  TuplePrefix<std::tuple_size<MatcherTuple>::value>::ExplainMatchFailuresTo(
      matchers, values, os);
}

// TransformTupleValues and its helper.
//
// TransformTupleValuesHelper hides the internal machinery that
// TransformTupleValues uses to implement a tuple traversal.
template <typename Tuple, typename Func, typename OutIter>
class TransformTupleValuesHelper {
 private:
  typedef ::std::tuple_size<Tuple> TupleSize;

 public:
  // For each member of tuple 't', taken in order, evaluates '*out++ = f(t)'.
  // Returns the final value of 'out' in case the caller needs it.
  static OutIter Run(Func f, const Tuple& t, OutIter out) {
    return IterateOverTuple<Tuple, TupleSize::value>()(f, t, out);
  }

 private:
  template <typename Tup, size_t kRemainingSize>
  struct IterateOverTuple {
    OutIter operator()(Func f, const Tup& t, OutIter out) const {
      *out++ = f(::std::get<TupleSize::value - kRemainingSize>(t));
      return IterateOverTuple<Tup, kRemainingSize - 1>()(f, t, out);
    }
  };
  template <typename Tup>
  struct IterateOverTuple<Tup, 0> {
    OutIter operator()(Func /* f */, const Tup& /* t */, OutIter out) const {
      return out;
    }
  };
};

// Successively invokes 'f(element)' on each element of the tuple 't',
// appending each result to the 'out' iterator. Returns the final value
// of 'out'.
template <typename Tuple, typename Func, typename OutIter>
OutIter TransformTupleValues(Func f, const Tuple& t, OutIter out) {
  return TransformTupleValuesHelper<Tuple, Func, OutIter>::Run(f, t, out);
}

// Implements _, a matcher that matches any value of any
// type.  This is a polymorphic matcher, so we need a template type
// conversion operator to make it appearing as a Matcher<T> for any
// type T.
class AnythingMatcher {
 public:
  using is_gtest_matcher = void;

  template <typename T>
  bool MatchAndExplain(const T& /* x */, std::ostream* /* listener */) const {
    return true;
  }
  void DescribeTo(std::ostream* os) const { *os << "is anything"; }
  void DescribeNegationTo(::std::ostream* os) const {
    // This is mostly for completeness' sake, as it's not very useful
    // to write Not(A<bool>()).  However we cannot completely rule out
    // such a possibility, and it doesn't hurt to be prepared.
    *os << "never matches";
  }
};

// Implements the polymorphic IsNull() matcher, which matches any raw or smart
// pointer that is NULL.
class IsNullMatcher {
 public:
  template <typename Pointer>
  bool MatchAndExplain(const Pointer& p,
                       MatchResultListener* /* listener */) const {
    return p == nullptr;
  }

  void DescribeTo(::std::ostream* os) const { *os << "is NULL"; }
  void DescribeNegationTo(::std::ostream* os) const { *os << "isn't NULL"; }
};

// Implements the polymorphic NotNull() matcher, which matches any raw or smart
// pointer that is not NULL.
class NotNullMatcher {
 public:
  template <typename Pointer>
  bool MatchAndExplain(const Pointer& p,
                       MatchResultListener* /* listener */) const {
    return p != nullptr;
  }

  void DescribeTo(::std::ostream* os) const { *os << "isn't NULL"; }
  void DescribeNegationTo(::std::ostream* os) const { *os << "is NULL"; }
};

// Ref(variable) matches any argument that is a reference to
// 'variable'.  This matcher is polymorphic as it can match any
// super type of the type of 'variable'.
//
// The RefMatcher template class implements Ref(variable).  It can
// only be instantiated with a reference type.  This prevents a user
// from mistakenly using Ref(x) to match a non-reference function
// argument.  For example, the following will righteously cause a
// compiler error:
//
//   int n;
//   Matcher<int> m1 = Ref(n);   // This won't compile.
//   Matcher<int&> m2 = Ref(n);  // This will compile.
template <typename T>
class RefMatcher;

template <typename T>
class RefMatcher<T&> {
  // Google Mock is a generic framework and thus needs to support
  // mocking any function types, including those that take non-const
  // reference arguments.  Therefore the template parameter T (and
  // Super below) can be instantiated to either a const type or a
  // non-const type.
 public:
  // RefMatcher() takes a T& instead of const T&, as we want the
  // compiler to catch using Ref(const_value) as a matcher for a
  // non-const reference.
  explicit RefMatcher(T& x) : object_(x) {}  // NOLINT

  template <typename Super>
  operator Matcher<Super&>() const {
    // By passing object_ (type T&) to Impl(), which expects a Super&,
    // we make sure that Super is a super type of T.  In particular,
    // this catches using Ref(const_value) as a matcher for a
    // non-const reference, as you cannot implicitly convert a const
    // reference to a non-const reference.
    return MakeMatcher(new Impl<Super>(object_));
  }

 private:
  template <typename Super>
  class Impl : public MatcherInterface<Super&> {
   public:
    explicit Impl(Super& x) : object_(x) {}  // NOLINT

    // MatchAndExplain() takes a Super& (as opposed to const Super&)
    // in order to match the interface MatcherInterface<Super&>.
    bool MatchAndExplain(Super& x,
                         MatchResultListener* listener) const override {
      *listener << "which is located @" << static_cast<const void*>(&x);
      return &x == &object_;
    }

    void DescribeTo(::std::ostream* os) const override {
      *os << "references the variable ";
      UniversalPrinter<Super&>::Print(object_, os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "does not reference the variable ";
      UniversalPrinter<Super&>::Print(object_, os);
    }

   private:
    const Super& object_;
  };

  T& object_;
};

// Polymorphic helper functions for narrow and wide string matchers.
inline bool CaseInsensitiveCStringEquals(const char* lhs, const char* rhs) {
  return String::CaseInsensitiveCStringEquals(lhs, rhs);
}

inline bool CaseInsensitiveCStringEquals(const wchar_t* lhs,
                                         const wchar_t* rhs) {
  return String::CaseInsensitiveWideCStringEquals(lhs, rhs);
}

// String comparison for narrow or wide strings that can have embedded NUL
// characters.
template <typename StringType>
bool CaseInsensitiveStringEquals(const StringType& s1, const StringType& s2) {
  // Are the heads equal?
  if (!CaseInsensitiveCStringEquals(s1.c_str(), s2.c_str())) {
    return false;
  }

  // Skip the equal heads.
  const typename StringType::value_type nul = 0;
  const size_t i1 = s1.find(nul), i2 = s2.find(nul);

  // Are we at the end of either s1 or s2?
  if (i1 == StringType::npos || i2 == StringType::npos) {
    return i1 == i2;
  }

  // Are the tails equal?
  return CaseInsensitiveStringEquals(s1.substr(i1 + 1), s2.substr(i2 + 1));
}

// String matchers.

// Implements equality-based string matchers like StrEq, StrCaseNe, and etc.
template <typename StringType>
class StrEqualityMatcher {
 public:
  StrEqualityMatcher(StringType str, bool expect_eq, bool case_sensitive)
      : string_(std::move(str)),
        expect_eq_(expect_eq),
        case_sensitive_(case_sensitive) {}

#if GTEST_INTERNAL_HAS_STRING_VIEW
  bool MatchAndExplain(const internal::StringView& s,
                       MatchResultListener* listener) const {
    // This should fail to compile if StringView is used with wide
    // strings.
    const StringType& str = std::string(s);
    return MatchAndExplain(str, listener);
  }
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW

  // Accepts pointer types, particularly:
  //   const char*
  //   char*
  //   const wchar_t*
  //   wchar_t*
  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    if (s == nullptr) {
      return !expect_eq_;
    }
    return MatchAndExplain(StringType(s), listener);
  }

  // Matches anything that can convert to StringType.
  //
  // This is a template, not just a plain function with const StringType&,
  // because StringView has some interfering non-explicit constructors.
  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener* /* listener */) const {
    const StringType s2(s);
    const bool eq = case_sensitive_ ? s2 == string_
                                    : CaseInsensitiveStringEquals(s2, string_);
    return expect_eq_ == eq;
  }

  void DescribeTo(::std::ostream* os) const {
    DescribeToHelper(expect_eq_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    DescribeToHelper(!expect_eq_, os);
  }

 private:
  void DescribeToHelper(bool expect_eq, ::std::ostream* os) const {
    *os << (expect_eq ? "is " : "isn't ");
    *os << "equal to ";
    if (!case_sensitive_) {
      *os << "(ignoring case) ";
    }
    UniversalPrint(string_, os);
  }

  const StringType string_;
  const bool expect_eq_;
  const bool case_sensitive_;
};

// Implements the polymorphic HasSubstr(substring) matcher, which
// can be used as a Matcher<T> as long as T can be converted to a
// string.
template <typename StringType>
class HasSubstrMatcher {
 public:
  explicit HasSubstrMatcher(const StringType& substring)
      : substring_(substring) {}

#if GTEST_INTERNAL_HAS_STRING_VIEW
  bool MatchAndExplain(const internal::StringView& s,
                       MatchResultListener* listener) const {
    // This should fail to compile if StringView is used with wide
    // strings.
    const StringType& str = std::string(s);
    return MatchAndExplain(str, listener);
  }
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW

  // Accepts pointer types, particularly:
  //   const char*
  //   char*
  //   const wchar_t*
  //   wchar_t*
  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    return s != nullptr && MatchAndExplain(StringType(s), listener);
  }

  // Matches anything that can convert to StringType.
  //
  // This is a template, not just a plain function with const StringType&,
  // because StringView has some interfering non-explicit constructors.
  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener* /* listener */) const {
    return StringType(s).find(substring_) != StringType::npos;
  }

  // Describes what this matcher matches.
  void DescribeTo(::std::ostream* os) const {
    *os << "has substring ";
    UniversalPrint(substring_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "has no substring ";
    UniversalPrint(substring_, os);
  }

 private:
  const StringType substring_;
};

// Implements the polymorphic StartsWith(substring) matcher, which
// can be used as a Matcher<T> as long as T can be converted to a
// string.
template <typename StringType>
class StartsWithMatcher {
 public:
  explicit StartsWithMatcher(const StringType& prefix) : prefix_(prefix) {}

#if GTEST_INTERNAL_HAS_STRING_VIEW
  bool MatchAndExplain(const internal::StringView& s,
                       MatchResultListener* listener) const {
    // This should fail to compile if StringView is used with wide
    // strings.
    const StringType& str = std::string(s);
    return MatchAndExplain(str, listener);
  }
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW

  // Accepts pointer types, particularly:
  //   const char*
  //   char*
  //   const wchar_t*
  //   wchar_t*
  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    return s != nullptr && MatchAndExplain(StringType(s), listener);
  }

  // Matches anything that can convert to StringType.
  //
  // This is a template, not just a plain function with const StringType&,
  // because StringView has some interfering non-explicit constructors.
  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener* /* listener */) const {
    const StringType s2(s);
    return s2.length() >= prefix_.length() &&
           s2.substr(0, prefix_.length()) == prefix_;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "starts with ";
    UniversalPrint(prefix_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't start with ";
    UniversalPrint(prefix_, os);
  }

 private:
  const StringType prefix_;
};

// Implements the polymorphic EndsWith(substring) matcher, which
// can be used as a Matcher<T> as long as T can be converted to a
// string.
template <typename StringType>
class EndsWithMatcher {
 public:
  explicit EndsWithMatcher(const StringType& suffix) : suffix_(suffix) {}

#if GTEST_INTERNAL_HAS_STRING_VIEW
  bool MatchAndExplain(const internal::StringView& s,
                       MatchResultListener* listener) const {
    // This should fail to compile if StringView is used with wide
    // strings.
    const StringType& str = std::string(s);
    return MatchAndExplain(str, listener);
  }
#endif  // GTEST_INTERNAL_HAS_STRING_VIEW

  // Accepts pointer types, particularly:
  //   const char*
  //   char*
  //   const wchar_t*
  //   wchar_t*
  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    return s != nullptr && MatchAndExplain(StringType(s), listener);
  }

  // Matches anything that can convert to StringType.
  //
  // This is a template, not just a plain function with const StringType&,
  // because StringView has some interfering non-explicit constructors.
  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener* /* listener */) const {
    const StringType s2(s);
    return s2.length() >= suffix_.length() &&
           s2.substr(s2.length() - suffix_.length()) == suffix_;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "ends with ";
    UniversalPrint(suffix_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't end with ";
    UniversalPrint(suffix_, os);
  }

 private:
  const StringType suffix_;
};

// Implements the polymorphic WhenBase64Unescaped(matcher) matcher, which can be
// used as a Matcher<T> as long as T can be converted to a string.
class WhenBase64UnescapedMatcher {
 public:
  using is_gtest_matcher = void;

  explicit WhenBase64UnescapedMatcher(
      const Matcher<const std::string&>& internal_matcher)
      : internal_matcher_(internal_matcher) {}

  // Matches anything that can convert to std::string.
  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener* listener) const {
    const std::string s2(s);  // NOLINT (needed for working with string_view).
    std::string unescaped;
    if (!internal::Base64Unescape(s2, &unescaped)) {
      if (listener != nullptr) {
        *listener << "is not a valid base64 escaped string";
      }
      return false;
    }
    return MatchPrintAndExplain(unescaped, internal_matcher_, listener);
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "matches after Base64Unescape ";
    internal_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "does not match after Base64Unescape ";
    internal_matcher_.DescribeTo(os);
  }

 private:
  const Matcher<const std::string&> internal_matcher_;
};

// Implements a matcher that compares the two fields of a 2-tuple
// using one of the ==, <=, <, etc, operators.  The two fields being
// compared don't have to have the same type.
//
// The matcher defined here is polymorphic (for example, Eq() can be
// used to match a std::tuple<int, short>, a std::tuple<const long&, double>,
// etc).  Therefore we use a template type conversion operator in the
// implementation.
template <typename D, typename Op>
class PairMatchBase {
 public:
  template <typename T1, typename T2>
  operator Matcher<::std::tuple<T1, T2>>() const {
    return Matcher<::std::tuple<T1, T2>>(new Impl<const ::std::tuple<T1, T2>&>);
  }
  template <typename T1, typename T2>
  operator Matcher<const ::std::tuple<T1, T2>&>() const {
    return MakeMatcher(new Impl<const ::std::tuple<T1, T2>&>);
  }

 private:
  static ::std::ostream& GetDesc(::std::ostream& os) {  // NOLINT
    return os << D::Desc();
  }

  template <typename Tuple>
  class Impl : public MatcherInterface<Tuple> {
   public:
    bool MatchAndExplain(Tuple args,
                         MatchResultListener* /* listener */) const override {
      return Op()(::std::get<0>(args), ::std::get<1>(args));
    }
    void DescribeTo(::std::ostream* os) const override {
      *os << "are " << GetDesc;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "aren't " << GetDesc;
    }
  };
};

class Eq2Matcher : public PairMatchBase<Eq2Matcher, std::equal_to<>> {
 public:
  static const char* Desc() { return "an equal pair"; }
};
class Ne2Matcher : public PairMatchBase<Ne2Matcher, std::not_equal_to<>> {
 public:
  static const char* Desc() { return "an unequal pair"; }
};
class Lt2Matcher : public PairMatchBase<Lt2Matcher, std::less<>> {
 public:
  static const char* Desc() { return "a pair where the first < the second"; }
};
class Gt2Matcher : public PairMatchBase<Gt2Matcher, std::greater<>> {
 public:
  static const char* Desc() { return "a pair where the first > the second"; }
};
class Le2Matcher : public PairMatchBase<Le2Matcher, std::less_equal<>> {
 public:
  static const char* Desc() { return "a pair where the first <= the second"; }
};
class Ge2Matcher : public PairMatchBase<Ge2Matcher, std::greater_equal<>> {
 public:
  static const char* Desc() { return "a pair where the first >= the second"; }
};

// Implements the Not(...) matcher for a particular argument type T.
// We do not nest it inside the NotMatcher class template, as that
// will prevent different instantiations of NotMatcher from sharing
// the same NotMatcherImpl<T> class.
template <typename T>
class NotMatcherImpl : public MatcherInterface<const T&> {
 public:
  explicit NotMatcherImpl(const Matcher<T>& matcher) : matcher_(matcher) {}

  bool MatchAndExplain(const T& x,
                       MatchResultListener* listener) const override {
    return !matcher_.MatchAndExplain(x, listener);
  }

  void DescribeTo(::std::ostream* os) const override {
    matcher_.DescribeNegationTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    matcher_.DescribeTo(os);
  }

 private:
  const Matcher<T> matcher_;
};

// Implements the Not(m) matcher, which matches a value that doesn't
// match matcher m.
template <typename InnerMatcher>
class NotMatcher {
 public:
  explicit NotMatcher(InnerMatcher matcher) : matcher_(matcher) {}

  // This template type conversion operator allows Not(m) to be used
  // to match any type m can match.
  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new NotMatcherImpl<T>(SafeMatcherCast<T>(matcher_)));
  }

 private:
  InnerMatcher matcher_;
};

// Implements the AllOf(m1, m2) matcher for a particular argument type
// T. We do not nest it inside the BothOfMatcher class template, as
// that will prevent different instantiations of BothOfMatcher from
// sharing the same BothOfMatcherImpl<T> class.
template <typename T>
class AllOfMatcherImpl : public MatcherInterface<const T&> {
 public:
  explicit AllOfMatcherImpl(std::vector<Matcher<T>> matchers)
      : matchers_(std::move(matchers)) {}

  void DescribeTo(::std::ostream* os) const override {
    *os << "(";
    for (size_t i = 0; i < matchers_.size(); ++i) {
      if (i != 0) *os << ") and (";
      matchers_[i].DescribeTo(os);
    }
    *os << ")";
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "(";
    for (size_t i = 0; i < matchers_.size(); ++i) {
      if (i != 0) *os << ") or (";
      matchers_[i].DescribeNegationTo(os);
    }
    *os << ")";
  }

  bool MatchAndExplain(const T& x,
                       MatchResultListener* listener) const override {
    // If either matcher1_ or matcher2_ doesn't match x, we only need
    // to explain why one of them fails.
    std::string all_match_result;

    for (size_t i = 0; i < matchers_.size(); ++i) {
      StringMatchResultListener slistener;
      if (matchers_[i].MatchAndExplain(x, &slistener)) {
        if (all_match_result.empty()) {
          all_match_result = slistener.str();
        } else {
          std::string result = slistener.str();
          if (!result.empty()) {
            all_match_result += ", and ";
            all_match_result += result;
          }
        }
      } else {
        *listener << slistener.str();
        return false;
      }
    }

    // Otherwise we need to explain why *both* of them match.
    *listener << all_match_result;
    return true;
  }

 private:
  const std::vector<Matcher<T>> matchers_;
};

// VariadicMatcher is used for the variadic implementation of
// AllOf(m_1, m_2, ...) and AnyOf(m_1, m_2, ...).
// CombiningMatcher<T> is used to recursively combine the provided matchers
// (of type Args...).
template <template <typename T> class CombiningMatcher, typename... Args>
class VariadicMatcher {
 public:
  VariadicMatcher(const Args&... matchers)  // NOLINT
      : matchers_(matchers...) {
    static_assert(sizeof...(Args) > 0, "Must have at least one matcher.");
  }

  VariadicMatcher(const VariadicMatcher&) = default;
  VariadicMatcher& operator=(const VariadicMatcher&) = delete;

  // This template type conversion operator allows an
  // VariadicMatcher<Matcher1, Matcher2...> object to match any type that
  // all of the provided matchers (Matcher1, Matcher2, ...) can match.
  template <typename T>
  operator Matcher<T>() const {
    std::vector<Matcher<T>> values;
    CreateVariadicMatcher<T>(&values, std::integral_constant<size_t, 0>());
    return Matcher<T>(new CombiningMatcher<T>(std::move(values)));
  }

 private:
  template <typename T, size_t I>
  void CreateVariadicMatcher(std::vector<Matcher<T>>* values,
                             std::integral_constant<size_t, I>) const {
    values->push_back(SafeMatcherCast<T>(std::get<I>(matchers_)));
    CreateVariadicMatcher<T>(values, std::integral_constant<size_t, I + 1>());
  }

  template <typename T>
  void CreateVariadicMatcher(
      std::vector<Matcher<T>>*,
      std::integral_constant<size_t, sizeof...(Args)>) const {}

  std::tuple<Args...> matchers_;
};

template <typename... Args>
using AllOfMatcher = VariadicMatcher<AllOfMatcherImpl, Args...>;

// Implements the AnyOf(m1, m2) matcher for a particular argument type
// T.  We do not nest it inside the AnyOfMatcher class template, as
// that will prevent different instantiations of AnyOfMatcher from
// sharing the same EitherOfMatcherImpl<T> class.
template <typename T>
class AnyOfMatcherImpl : public MatcherInterface<const T&> {
 public:
  explicit AnyOfMatcherImpl(std::vector<Matcher<T>> matchers)
      : matchers_(std::move(matchers)) {}

  void DescribeTo(::std::ostream* os) const override {
    *os << "(";
    for (size_t i = 0; i < matchers_.size(); ++i) {
      if (i != 0) *os << ") or (";
      matchers_[i].DescribeTo(os);
    }
    *os << ")";
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "(";
    for (size_t i = 0; i < matchers_.size(); ++i) {
      if (i != 0) *os << ") and (";
      matchers_[i].DescribeNegationTo(os);
    }
    *os << ")";
  }

  bool MatchAndExplain(const T& x,
                       MatchResultListener* listener) const override {
    std::string no_match_result;

    // If either matcher1_ or matcher2_ matches x, we just need to
    // explain why *one* of them matches.
    for (size_t i = 0; i < matchers_.size(); ++i) {
      StringMatchResultListener slistener;
      if (matchers_[i].MatchAndExplain(x, &slistener)) {
        *listener << slistener.str();
        return true;
      } else {
        if (no_match_result.empty()) {
          no_match_result = slistener.str();
        } else {
          std::string result = slistener.str();
          if (!result.empty()) {
            no_match_result += ", and ";
            no_match_result += result;
          }
        }
      }
    }

    // Otherwise we need to explain why *both* of them fail.
    *listener << no_match_result;
    return false;
  }

 private:
  const std::vector<Matcher<T>> matchers_;
};

// AnyOfMatcher is used for the variadic implementation of AnyOf(m_1, m_2, ...).
template <typename... Args>
using AnyOfMatcher = VariadicMatcher<AnyOfMatcherImpl, Args...>;

// ConditionalMatcher is the implementation of Conditional(cond, m1, m2)
template <typename MatcherTrue, typename MatcherFalse>
class ConditionalMatcher {
 public:
  ConditionalMatcher(bool condition, MatcherTrue matcher_true,
                     MatcherFalse matcher_false)
      : condition_(condition),
        matcher_true_(std::move(matcher_true)),
        matcher_false_(std::move(matcher_false)) {}

  template <typename T>
  operator Matcher<T>() const {  // NOLINT(runtime/explicit)
    return condition_ ? SafeMatcherCast<T>(matcher_true_)
                      : SafeMatcherCast<T>(matcher_false_);
  }

 private:
  bool condition_;
  MatcherTrue matcher_true_;
  MatcherFalse matcher_false_;
};

// Wrapper for implementation of Any/AllOfArray().
template <template <class> class MatcherImpl, typename T>
class SomeOfArrayMatcher {
 public:
  // Constructs the matcher from a sequence of element values or
  // element matchers.
  template <typename Iter>
  SomeOfArrayMatcher(Iter first, Iter last) : matchers_(first, last) {}

  template <typename U>
  operator Matcher<U>() const {  // NOLINT
    using RawU = typename std::decay<U>::type;
    std::vector<Matcher<RawU>> matchers;
    matchers.reserve(matchers_.size());
    for (const auto& matcher : matchers_) {
      matchers.push_back(MatcherCast<RawU>(matcher));
    }
    return Matcher<U>(new MatcherImpl<RawU>(std::move(matchers)));
  }

 private:
  const ::std::vector<T> matchers_;
};

template <typename T>
using AllOfArrayMatcher = SomeOfArrayMatcher<AllOfMatcherImpl, T>;

template <typename T>
using AnyOfArrayMatcher = SomeOfArrayMatcher<AnyOfMatcherImpl, T>;

// Used for implementing Truly(pred), which turns a predicate into a
// matcher.
template <typename Predicate>
class TrulyMatcher {
 public:
  explicit TrulyMatcher(Predicate pred) : predicate_(pred) {}

  // This method template allows Truly(pred) to be used as a matcher
  // for type T where T is the argument type of predicate 'pred'.  The
  // argument is passed by reference as the predicate may be
  // interested in the address of the argument.
  template <typename T>
  bool MatchAndExplain(T& x,  // NOLINT
                       MatchResultListener* listener) const {
    // Without the if-statement, MSVC sometimes warns about converting
    // a value to bool (warning 4800).
    //
    // We cannot write 'return !!predicate_(x);' as that doesn't work
    // when predicate_(x) returns a class convertible to bool but
    // having no operator!().
    if (predicate_(x)) return true;
    *listener << "didn't satisfy the given predicate";
    return false;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "satisfies the given predicate";
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't satisfy the given predicate";
  }

 private:
  Predicate predicate_;
};

// Used for implementing Matches(matcher), which turns a matcher into
// a predicate.
template <typename M>
class MatcherAsPredicate {
 public:
  explicit MatcherAsPredicate(M matcher) : matcher_(matcher) {}

  // This template operator() allows Matches(m) to be used as a
  // predicate on type T where m is a matcher on type T.
  //
  // The argument x is passed by reference instead of by value, as
  // some matcher may be interested in its address (e.g. as in
  // Matches(Ref(n))(x)).
  template <typename T>
  bool operator()(const T& x) const {
    // We let matcher_ commit to a particular type here instead of
    // when the MatcherAsPredicate object was constructed.  This
    // allows us to write Matches(m) where m is a polymorphic matcher
    // (e.g. Eq(5)).
    //
    // If we write Matcher<T>(matcher_).Matches(x) here, it won't
    // compile when matcher_ has type Matcher<const T&>; if we write
    // Matcher<const T&>(matcher_).Matches(x) here, it won't compile
    // when matcher_ has type Matcher<T>; if we just write
    // matcher_.Matches(x), it won't compile when matcher_ is
    // polymorphic, e.g. Eq(5).
    //
    // MatcherCast<const T&>() is necessary for making the code work
    // in all of the above situations.
    return MatcherCast<const T&>(matcher_).Matches(x);
  }

 private:
  M matcher_;
};

// For implementing ASSERT_THAT() and EXPECT_THAT().  The template
// argument M must be a type that can be converted to a matcher.
template <typename M>
class PredicateFormatterFromMatcher {
 public:
  explicit PredicateFormatterFromMatcher(M m) : matcher_(std::move(m)) {}

  // This template () operator allows a PredicateFormatterFromMatcher
  // object to act as a predicate-formatter suitable for using with
  // Google Test's EXPECT_PRED_FORMAT1() macro.
  template <typename T>
  AssertionResult operator()(const char* value_text, const T& x) const {
    // We convert matcher_ to a Matcher<const T&> *now* instead of
    // when the PredicateFormatterFromMatcher object was constructed,
    // as matcher_ may be polymorphic (e.g. NotNull()) and we won't
    // know which type to instantiate it to until we actually see the
    // type of x here.
    //
    // We write SafeMatcherCast<const T&>(matcher_) instead of
    // Matcher<const T&>(matcher_), as the latter won't compile when
    // matcher_ has type Matcher<T> (e.g. An<int>()).
    // We don't write MatcherCast<const T&> either, as that allows
    // potentially unsafe downcasting of the matcher argument.
    const Matcher<const T&> matcher = SafeMatcherCast<const T&>(matcher_);

    // The expected path here is that the matcher should match (i.e. that most
    // tests pass) so optimize for this case.
    if (matcher.Matches(x)) {
      return AssertionSuccess();
    }

    ::std::stringstream ss;
    ss << "Value of: " << value_text << "\n"
       << "Expected: ";
    matcher.DescribeTo(&ss);

    // Rerun the matcher to "PrintAndExplain" the failure.
    StringMatchResultListener listener;
    if (MatchPrintAndExplain(x, matcher, &listener)) {
      ss << "\n  The matcher failed on the initial attempt; but passed when "
            "rerun to generate the explanation.";
    }
    ss << "\n  Actual: " << listener.str();
    return AssertionFailure() << ss.str();
  }

 private:
  const M matcher_;
};

// A helper function for converting a matcher to a predicate-formatter
// without the user needing to explicitly write the type.  This is
// used for implementing ASSERT_THAT() and EXPECT_THAT().
// Implementation detail: 'matcher' is received by-value to force decaying.
template <typename M>
inline PredicateFormatterFromMatcher<M> MakePredicateFormatterFromMatcher(
    M matcher) {
  return PredicateFormatterFromMatcher<M>(std::move(matcher));
}

// Implements the polymorphic IsNan() matcher, which matches any floating type
// value that is Nan.
class IsNanMatcher {
 public:
  template <typename FloatType>
  bool MatchAndExplain(const FloatType& f,
                       MatchResultListener* /* listener */) const {
    return (::std::isnan)(f);
  }

  void DescribeTo(::std::ostream* os) const { *os << "is NaN"; }
  void DescribeNegationTo(::std::ostream* os) const { *os << "isn't NaN"; }
};

// Implements the polymorphic floating point equality matcher, which matches
// two float values using ULP-based approximation or, optionally, a
// user-specified epsilon.  The template is meant to be instantiated with
// FloatType being either float or double.
template <typename FloatType>
class FloatingEqMatcher {
 public:
  // Constructor for FloatingEqMatcher.
  // The matcher's input will be compared with expected.  The matcher treats two
  // NANs as equal if nan_eq_nan is true.  Otherwise, under IEEE standards,
  // equality comparisons between NANs will always return false.  We specify a
  // negative max_abs_error_ term to indicate that ULP-based approximation will
  // be used for comparison.
  FloatingEqMatcher(FloatType expected, bool nan_eq_nan)
      : expected_(expected), nan_eq_nan_(nan_eq_nan), max_abs_error_(-1) {}

  // Constructor that supports a user-specified max_abs_error that will be used
  // for comparison instead of ULP-based approximation.  The max absolute
  // should be non-negative.
  FloatingEqMatcher(FloatType expected, bool nan_eq_nan,
                    FloatType max_abs_error)
      : expected_(expected),
        nan_eq_nan_(nan_eq_nan),
        max_abs_error_(max_abs_error) {
    GTEST_CHECK_(max_abs_error >= 0)
        << ", where max_abs_error is" << max_abs_error;
  }

  // Implements floating point equality matcher as a Matcher<T>.
  template <typename T>
  class Impl : public MatcherInterface<T> {
   public:
    Impl(FloatType expected, bool nan_eq_nan, FloatType max_abs_error)
        : expected_(expected),
          nan_eq_nan_(nan_eq_nan),
          max_abs_error_(max_abs_error) {}

    bool MatchAndExplain(T value,
                         MatchResultListener* listener) const override {
      const FloatingPoint<FloatType> actual(value), expected(expected_);

      // Compares NaNs first, if nan_eq_nan_ is true.
      if (actual.is_nan() || expected.is_nan()) {
        if (actual.is_nan() && expected.is_nan()) {
          return nan_eq_nan_;
        }
        // One is nan; the other is not nan.
        return false;
      }
      if (HasMaxAbsError()) {
        // We perform an equality check so that inf will match inf, regardless
        // of error bounds.  If the result of value - expected_ would result in
        // overflow or if either value is inf, the default result is infinity,
        // which should only match if max_abs_error_ is also infinity.
        if (value == expected_) {
          return true;
        }

        const FloatType diff = value - expected_;
        if (::std::fabs(diff) <= max_abs_error_) {
          return true;
        }

        if (listener->IsInterested()) {
          *listener << "which is " << diff << " from " << expected_;
        }
        return false;
      } else {
        return actual.AlmostEquals(expected);
      }
    }

    void DescribeTo(::std::ostream* os) const override {
      // os->precision() returns the previously set precision, which we
      // store to restore the ostream to its original configuration
      // after outputting.
      const ::std::streamsize old_precision =
          os->precision(::std::numeric_limits<FloatType>::digits10 + 2);
      if (FloatingPoint<FloatType>(expected_).is_nan()) {
        if (nan_eq_nan_) {
          *os << "is NaN";
        } else {
          *os << "never matches";
        }
      } else {
        *os << "is approximately " << expected_;
        if (HasMaxAbsError()) {
          *os << " (absolute error <= " << max_abs_error_ << ")";
        }
      }
      os->precision(old_precision);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      // As before, get original precision.
      const ::std::streamsize old_precision =
          os->precision(::std::numeric_limits<FloatType>::digits10 + 2);
      if (FloatingPoint<FloatType>(expected_).is_nan()) {
        if (nan_eq_nan_) {
          *os << "isn't NaN";
        } else {
          *os << "is anything";
        }
      } else {
        *os << "isn't approximately " << expected_;
        if (HasMaxAbsError()) {
          *os << " (absolute error > " << max_abs_error_ << ")";
        }
      }
      // Restore original precision.
      os->precision(old_precision);
    }

   private:
    bool HasMaxAbsError() const { return max_abs_error_ >= 0; }

    const FloatType expected_;
    const bool nan_eq_nan_;
    // max_abs_error will be used for value comparison when >= 0.
    const FloatType max_abs_error_;
  };

  // The following 3 type conversion operators allow FloatEq(expected) and
  // NanSensitiveFloatEq(expected) to be used as a Matcher<float>, a
  // Matcher<const float&>, or a Matcher<float&>, but nothing else.
  operator Matcher<FloatType>() const {
    return MakeMatcher(
        new Impl<FloatType>(expected_, nan_eq_nan_, max_abs_error_));
  }

  operator Matcher<const FloatType&>() const {
    return MakeMatcher(
        new Impl<const FloatType&>(expected_, nan_eq_nan_, max_abs_error_));
  }

  operator Matcher<FloatType&>() const {
    return MakeMatcher(
        new Impl<FloatType&>(expected_, nan_eq_nan_, max_abs_error_));
  }

 private:
  const FloatType expected_;
  const bool nan_eq_nan_;
  // max_abs_error will be used for value comparison when >= 0.
  const FloatType max_abs_error_;
};

// A 2-tuple ("binary") wrapper around FloatingEqMatcher:
// FloatingEq2Matcher() matches (x, y) by matching FloatingEqMatcher(x, false)
// against y, and FloatingEq2Matcher(e) matches FloatingEqMatcher(x, false, e)
// against y. The former implements "Eq", the latter "Near". At present, there
// is no version that compares NaNs as equal.
template <typename FloatType>
class FloatingEq2Matcher {
 public:
  FloatingEq2Matcher() { Init(-1, false); }

  explicit FloatingEq2Matcher(bool nan_eq_nan) { Init(-1, nan_eq_nan); }

  explicit FloatingEq2Matcher(FloatType max_abs_error) {
    Init(max_abs_error, false);
  }

  FloatingEq2Matcher(FloatType max_abs_error, bool nan_eq_nan) {
    Init(max_abs_error, nan_eq_nan);
  }

  template <typename T1, typename T2>
  operator Matcher<::std::tuple<T1, T2>>() const {
    return MakeMatcher(
        new Impl<::std::tuple<T1, T2>>(max_abs_error_, nan_eq_nan_));
  }
  template <typename T1, typename T2>
  operator Matcher<const ::std::tuple<T1, T2>&>() const {
    return MakeMatcher(
        new Impl<const ::std::tuple<T1, T2>&>(max_abs_error_, nan_eq_nan_));
  }

 private:
  static ::std::ostream& GetDesc(::std::ostream& os) {  // NOLINT
    return os << "an almost-equal pair";
  }

  template <typename Tuple>
  class Impl : public MatcherInterface<Tuple> {
   public:
    Impl(FloatType max_abs_error, bool nan_eq_nan)
        : max_abs_error_(max_abs_error), nan_eq_nan_(nan_eq_nan) {}

    bool MatchAndExplain(Tuple args,
                         MatchResultListener* listener) const override {
      if (max_abs_error_ == -1) {
        FloatingEqMatcher<FloatType> fm(::std::get<0>(args), nan_eq_nan_);
        return static_cast<Matcher<FloatType>>(fm).MatchAndExplain(
            ::std::get<1>(args), listener);
      } else {
        FloatingEqMatcher<FloatType> fm(::std::get<0>(args), nan_eq_nan_,
                                        max_abs_error_);
        return static_cast<Matcher<FloatType>>(fm).MatchAndExplain(
            ::std::get<1>(args), listener);
      }
    }
    void DescribeTo(::std::ostream* os) const override {
      *os << "are " << GetDesc;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "aren't " << GetDesc;
    }

   private:
    FloatType max_abs_error_;
    const bool nan_eq_nan_;
  };

  void Init(FloatType max_abs_error_val, bool nan_eq_nan_val) {
    max_abs_error_ = max_abs_error_val;
    nan_eq_nan_ = nan_eq_nan_val;
  }
  FloatType max_abs_error_;
  bool nan_eq_nan_;
};

// Implements the Pointee(m) matcher for matching a pointer whose
// pointee matches matcher m.  The pointer can be either raw or smart.
template <typename InnerMatcher>
class PointeeMatcher {
 public:
  explicit PointeeMatcher(const InnerMatcher& matcher) : matcher_(matcher) {}

  // This type conversion operator template allows Pointee(m) to be
  // used as a matcher for any pointer type whose pointee type is
  // compatible with the inner matcher, where type Pointer can be
  // either a raw pointer or a smart pointer.
  //
  // The reason we do this instead of relying on
  // MakePolymorphicMatcher() is that the latter is not flexible
  // enough for implementing the DescribeTo() method of Pointee().
  template <typename Pointer>
  operator Matcher<Pointer>() const {
    return Matcher<Pointer>(new Impl<const Pointer&>(matcher_));
  }

 private:
  // The monomorphic implementation that works for a particular pointer type.
  template <typename Pointer>
  class Impl : public MatcherInterface<Pointer> {
   public:
    using Pointee =
        typename std::pointer_traits<GTEST_REMOVE_REFERENCE_AND_CONST_(
            Pointer)>::element_type;

    explicit Impl(const InnerMatcher& matcher)
        : matcher_(MatcherCast<const Pointee&>(matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "points to a value that ";
      matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "does not point to a value that ";
      matcher_.DescribeTo(os);
    }

    bool MatchAndExplain(Pointer pointer,
                         MatchResultListener* listener) const override {
      if (GetRawPointer(pointer) == nullptr) return false;

      *listener << "which points to ";
      return MatchPrintAndExplain(*pointer, matcher_, listener);
    }

   private:
    const Matcher<const Pointee&> matcher_;
  };

  const InnerMatcher matcher_;
};

// Implements the Pointer(m) matcher
// Implements the Pointer(m) matcher for matching a pointer that matches matcher
// m.  The pointer can be either raw or smart, and will match `m` against the
// raw pointer.
template <typename InnerMatcher>
class PointerMatcher {
 public:
  explicit PointerMatcher(const InnerMatcher& matcher) : matcher_(matcher) {}

  // This type conversion operator template allows Pointer(m) to be
  // used as a matcher for any pointer type whose pointer type is
  // compatible with the inner matcher, where type PointerType can be
  // either a raw pointer or a smart pointer.
  //
  // The reason we do this instead of relying on
  // MakePolymorphicMatcher() is that the latter is not flexible
  // enough for implementing the DescribeTo() method of Pointer().
  template <typename PointerType>
  operator Matcher<PointerType>() const {  // NOLINT
    return Matcher<PointerType>(new Impl<const PointerType&>(matcher_));
  }

 private:
  // The monomorphic implementation that works for a particular pointer type.
  template <typename PointerType>
  class Impl : public MatcherInterface<PointerType> {
   public:
    using Pointer =
        const typename std::pointer_traits<GTEST_REMOVE_REFERENCE_AND_CONST_(
            PointerType)>::element_type*;

    explicit Impl(const InnerMatcher& matcher)
        : matcher_(MatcherCast<Pointer>(matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "is a pointer that ";
      matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "is not a pointer that ";
      matcher_.DescribeTo(os);
    }

    bool MatchAndExplain(PointerType pointer,
                         MatchResultListener* listener) const override {
      *listener << "which is a pointer that ";
      Pointer p = GetRawPointer(pointer);
      return MatchPrintAndExplain(p, matcher_, listener);
    }

   private:
    Matcher<Pointer> matcher_;
  };

  const InnerMatcher matcher_;
};

#if GTEST_HAS_RTTI
// Implements the WhenDynamicCastTo<T>(m) matcher that matches a pointer or
// reference that matches inner_matcher when dynamic_cast<T> is applied.
// The result of dynamic_cast<To> is forwarded to the inner matcher.
// If To is a pointer and the cast fails, the inner matcher will receive NULL.
// If To is a reference and the cast fails, this matcher returns false
// immediately.
template <typename To>
class WhenDynamicCastToMatcherBase {
 public:
  explicit WhenDynamicCastToMatcherBase(const Matcher<To>& matcher)
      : matcher_(matcher) {}

  void DescribeTo(::std::ostream* os) const {
    GetCastTypeDescription(os);
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    GetCastTypeDescription(os);
    matcher_.DescribeNegationTo(os);
  }

 protected:
  const Matcher<To> matcher_;

  static std::string GetToName() { return GetTypeName<To>(); }

 private:
  static void GetCastTypeDescription(::std::ostream* os) {
    *os << "when dynamic_cast to " << GetToName() << ", ";
  }
};

// Primary template.
// To is a pointer. Cast and forward the result.
template <typename To>
class WhenDynamicCastToMatcher : public WhenDynamicCastToMatcherBase<To> {
 public:
  explicit WhenDynamicCastToMatcher(const Matcher<To>& matcher)
      : WhenDynamicCastToMatcherBase<To>(matcher) {}

  template <typename From>
  bool MatchAndExplain(From from, MatchResultListener* listener) const {
    To to = dynamic_cast<To>(from);
    return MatchPrintAndExplain(to, this->matcher_, listener);
  }
};

// Specialize for references.
// In this case we return false if the dynamic_cast fails.
template <typename To>
class WhenDynamicCastToMatcher<To&> : public WhenDynamicCastToMatcherBase<To&> {
 public:
  explicit WhenDynamicCastToMatcher(const Matcher<To&>& matcher)
      : WhenDynamicCastToMatcherBase<To&>(matcher) {}

  template <typename From>
  bool MatchAndExplain(From& from, MatchResultListener* listener) const {
    // We don't want an std::bad_cast here, so do the cast with pointers.
    To* to = dynamic_cast<To*>(&from);
    if (to == nullptr) {
      *listener << "which cannot be dynamic_cast to " << this->GetToName();
      return false;
    }
    return MatchPrintAndExplain(*to, this->matcher_, listener);
  }
};
#endif  // GTEST_HAS_RTTI

// Implements the Field() matcher for matching a field (i.e. member
// variable) of an object.
template <typename Class, typename FieldType>
class FieldMatcher {
 public:
  FieldMatcher(FieldType Class::*field,
               const Matcher<const FieldType&>& matcher)
      : field_(field), matcher_(matcher), whose_field_("whose given field ") {}

  FieldMatcher(const std::string& field_name, FieldType Class::*field,
               const Matcher<const FieldType&>& matcher)
      : field_(field),
        matcher_(matcher),
        whose_field_("whose field `" + field_name + "` ") {}

  void DescribeTo(::std::ostream* os) const {
    *os << "is an object " << whose_field_;
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is an object " << whose_field_;
    matcher_.DescribeNegationTo(os);
  }

  template <typename T>
  bool MatchAndExplain(const T& value, MatchResultListener* listener) const {
    // FIXME: The dispatch on std::is_pointer was introduced as a workaround for
    // a compiler bug, and can now be removed.
    return MatchAndExplainImpl(
        typename std::is_pointer<typename std::remove_const<T>::type>::type(),
        value, listener);
  }

 private:
  bool MatchAndExplainImpl(std::false_type /* is_not_pointer */,
                           const Class& obj,
                           MatchResultListener* listener) const {
    *listener << whose_field_ << "is ";
    return MatchPrintAndExplain(obj.*field_, matcher_, listener);
  }

  bool MatchAndExplainImpl(std::true_type /* is_pointer */, const Class* p,
                           MatchResultListener* listener) const {
    if (p == nullptr) return false;

    *listener << "which points to an object ";
    // Since *p has a field, it must be a class/struct/union type and
    // thus cannot be a pointer.  Therefore we pass false_type() as
    // the first argument.
    return MatchAndExplainImpl(std::false_type(), *p, listener);
  }

  const FieldType Class::*field_;
  const Matcher<const FieldType&> matcher_;

  // Contains either "whose given field " if the name of the field is unknown
  // or "whose field `name_of_field` " if the name is known.
  const std::string whose_field_;
};

// Implements the Property() matcher for matching a property
// (i.e. return value of a getter method) of an object.
//
// Property is a const-qualified member function of Class returning
// PropertyType.
template <typename Class, typename PropertyType, typename Property>
class PropertyMatcher {
 public:
  typedef const PropertyType& RefToConstProperty;

  PropertyMatcher(Property property, const Matcher<RefToConstProperty>& matcher)
      : property_(property),
        matcher_(matcher),
        whose_property_("whose given property ") {}

  PropertyMatcher(const std::string& property_name, Property property,
                  const Matcher<RefToConstProperty>& matcher)
      : property_(property),
        matcher_(matcher),
        whose_property_("whose property `" + property_name + "` ") {}

  void DescribeTo(::std::ostream* os) const {
    *os << "is an object " << whose_property_;
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is an object " << whose_property_;
    matcher_.DescribeNegationTo(os);
  }

  template <typename T>
  bool MatchAndExplain(const T& value, MatchResultListener* listener) const {
    return MatchAndExplainImpl(
        typename std::is_pointer<typename std::remove_const<T>::type>::type(),
        value, listener);
  }

 private:
  bool MatchAndExplainImpl(std::false_type /* is_not_pointer */,
                           const Class& obj,
                           MatchResultListener* listener) const {
    *listener << whose_property_ << "is ";
    // Cannot pass the return value (for example, int) to MatchPrintAndExplain,
    // which takes a non-const reference as argument.
    RefToConstProperty result = (obj.*property_)();
    return MatchPrintAndExplain(result, matcher_, listener);
  }

  bool MatchAndExplainImpl(std::true_type /* is_pointer */, const Class* p,
                           MatchResultListener* listener) const {
    if (p == nullptr) return false;

    *listener << "which points to an object ";
    // Since *p has a property method, it must be a class/struct/union
    // type and thus cannot be a pointer.  Therefore we pass
    // false_type() as the first argument.
    return MatchAndExplainImpl(std::false_type(), *p, listener);
  }

  Property property_;
  const Matcher<RefToConstProperty> matcher_;

  // Contains either "whose given property " if the name of the property is
  // unknown or "whose property `name_of_property` " if the name is known.
  const std::string whose_property_;
};

// Type traits specifying various features of different functors for ResultOf.
// The default template specifies features for functor objects.
template <typename Functor>
struct CallableTraits {
  typedef Functor StorageType;

  static void CheckIsValid(Functor /* functor */) {}

  template <typename T>
  static auto Invoke(Functor f, const T& arg) -> decltype(f(arg)) {
    return f(arg);
  }
};

// Specialization for function pointers.
template <typename ArgType, typename ResType>
struct CallableTraits<ResType (*)(ArgType)> {
  typedef ResType ResultType;
  typedef ResType (*StorageType)(ArgType);

  static void CheckIsValid(ResType (*f)(ArgType)) {
    GTEST_CHECK_(f != nullptr)
        << "NULL function pointer is passed into ResultOf().";
  }
  template <typename T>
  static ResType Invoke(ResType (*f)(ArgType), T arg) {
    return (*f)(arg);
  }
};

// Implements the ResultOf() matcher for matching a return value of a
// unary function of an object.
template <typename Callable, typename InnerMatcher>
class ResultOfMatcher {
 public:
  ResultOfMatcher(Callable callable, InnerMatcher matcher)
      : ResultOfMatcher(/*result_description=*/"", std::move(callable),
                        std::move(matcher)) {}

  ResultOfMatcher(const std::string& result_description, Callable callable,
                  InnerMatcher matcher)
      : result_description_(result_description),
        callable_(std::move(callable)),
        matcher_(std::move(matcher)) {
    CallableTraits<Callable>::CheckIsValid(callable_);
  }

  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(
        new Impl<const T&>(result_description_, callable_, matcher_));
  }

 private:
  typedef typename CallableTraits<Callable>::StorageType CallableStorageType;

  template <typename T>
  class Impl : public MatcherInterface<T> {
    using ResultType = decltype(CallableTraits<Callable>::template Invoke<T>(
        std::declval<CallableStorageType>(), std::declval<T>()));

   public:
    template <typename M>
    Impl(const std::string& result_description,
         const CallableStorageType& callable, const M& matcher)
        : result_description_(result_description),
          callable_(callable),
          matcher_(MatcherCast<ResultType>(matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      if (result_description_.empty()) {
        *os << "is mapped by the given callable to a value that ";
      } else {
        *os << "whose " << result_description_ << " ";
      }
      matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      if (result_description_.empty()) {
        *os << "is mapped by the given callable to a value that ";
      } else {
        *os << "whose " << result_description_ << " ";
      }
      matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(T obj, MatchResultListener* listener) const override {
      if (result_description_.empty()) {
        *listener << "which is mapped by the given callable to ";
      } else {
        *listener << "whose " << result_description_ << " is ";
      }
      // Cannot pass the return value directly to MatchPrintAndExplain, which
      // takes a non-const reference as argument.
      // Also, specifying template argument explicitly is needed because T could
      // be a non-const reference (e.g. Matcher<Uncopyable&>).
      ResultType result =
          CallableTraits<Callable>::template Invoke<T>(callable_, obj);
      return MatchPrintAndExplain(result, matcher_, listener);
    }

   private:
    const std::string result_description_;
    // Functors often define operator() as non-const method even though
    // they are actually stateless. But we need to use them even when
    // 'this' is a const pointer. It's the user's responsibility not to
    // use stateful callables with ResultOf(), which doesn't guarantee
    // how many times the callable will be invoked.
    mutable CallableStorageType callable_;
    const Matcher<ResultType> matcher_;
  };  // class Impl

  const std::string result_description_;
  const CallableStorageType callable_;
  const InnerMatcher matcher_;
};

// Implements a matcher that checks the size of an STL-style container.
template <typename SizeMatcher>
class SizeIsMatcher {
 public:
  explicit SizeIsMatcher(const SizeMatcher& size_matcher)
      : size_matcher_(size_matcher) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return Matcher<Container>(new Impl<const Container&>(size_matcher_));
  }

  template <typename Container>
  class Impl : public MatcherInterface<Container> {
   public:
    using SizeType = decltype(std::declval<Container>().size());
    explicit Impl(const SizeMatcher& size_matcher)
        : size_matcher_(MatcherCast<SizeType>(size_matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "has a size that ";
      size_matcher_.DescribeTo(os);
    }
    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "has a size that ";
      size_matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(Container container,
                         MatchResultListener* listener) const override {
      SizeType size = container.size();
      StringMatchResultListener size_listener;
      const bool result = size_matcher_.MatchAndExplain(size, &size_listener);
      *listener << "whose size " << size
                << (result ? " matches" : " doesn't match");
      PrintIfNotEmpty(size_listener.str(), listener->stream());
      return result;
    }

   private:
    const Matcher<SizeType> size_matcher_;
  };

 private:
  const SizeMatcher size_matcher_;
};

// Implements a matcher that checks the begin()..end() distance of an STL-style
// container.
template <typename DistanceMatcher>
class BeginEndDistanceIsMatcher {
 public:
  explicit BeginEndDistanceIsMatcher(const DistanceMatcher& distance_matcher)
      : distance_matcher_(distance_matcher) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return Matcher<Container>(new Impl<const Container&>(distance_matcher_));
  }

  template <typename Container>
  class Impl : public MatcherInterface<Container> {
   public:
    typedef internal::StlContainerView<GTEST_REMOVE_REFERENCE_AND_CONST_(
        Container)>
        ContainerView;
    typedef typename std::iterator_traits<
        typename ContainerView::type::const_iterator>::difference_type
        DistanceType;
    explicit Impl(const DistanceMatcher& distance_matcher)
        : distance_matcher_(MatcherCast<DistanceType>(distance_matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "distance between begin() and end() ";
      distance_matcher_.DescribeTo(os);
    }
    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "distance between begin() and end() ";
      distance_matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(Container container,
                         MatchResultListener* listener) const override {
      using std::begin;
      using std::end;
      DistanceType distance = std::distance(begin(container), end(container));
      StringMatchResultListener distance_listener;
      const bool result =
          distance_matcher_.MatchAndExplain(distance, &distance_listener);
      *listener << "whose distance between begin() and end() " << distance
                << (result ? " matches" : " doesn't match");
      PrintIfNotEmpty(distance_listener.str(), listener->stream());
      return result;
    }

   private:
    const Matcher<DistanceType> distance_matcher_;
  };

 private:
  const DistanceMatcher distance_matcher_;
};

// Implements an equality matcher for any STL-style container whose elements
// support ==. This matcher is like Eq(), but its failure explanations provide
// more detailed information that is useful when the container is used as a set.
// The failure message reports elements that are in one of the operands but not
// the other. The failure messages do not report duplicate or out-of-order
// elements in the containers (which don't properly matter to sets, but can
// occur if the containers are vectors or lists, for example).
//
// Uses the container's const_iterator, value_type, operator ==,
// begin(), and end().
template <typename Container>
class ContainerEqMatcher {
 public:
  typedef internal::StlContainerView<Container> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;

  static_assert(!std::is_const<Container>::value,
                "Container type must not be const");
  static_assert(!std::is_reference<Container>::value,
                "Container type must not be a reference");

  // We make a copy of expected in case the elements in it are modified
  // after this matcher is created.
  explicit ContainerEqMatcher(const Container& expected)
      : expected_(View::Copy(expected)) {}

  void DescribeTo(::std::ostream* os) const {
    *os << "equals ";
    UniversalPrint(expected_, os);
  }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "does not equal ";
    UniversalPrint(expected_, os);
  }

  template <typename LhsContainer>
  bool MatchAndExplain(const LhsContainer& lhs,
                       MatchResultListener* listener) const {
    typedef internal::StlContainerView<
        typename std::remove_const<LhsContainer>::type>
        LhsView;
    StlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
    if (lhs_stl_container == expected_) return true;

    ::std::ostream* const os = listener->stream();
    if (os != nullptr) {
      // Something is different. Check for extra values first.
      bool printed_header = false;
      for (auto it = lhs_stl_container.begin(); it != lhs_stl_container.end();
           ++it) {
        if (internal::ArrayAwareFind(expected_.begin(), expected_.end(), *it) ==
            expected_.end()) {
          if (printed_header) {
            *os << ", ";
          } else {
            *os << "which has these unexpected elements: ";
            printed_header = true;
          }
          UniversalPrint(*it, os);
        }
      }

      // Now check for missing values.
      bool printed_header2 = false;
      for (auto it = expected_.begin(); it != expected_.end(); ++it) {
        if (internal::ArrayAwareFind(lhs_stl_container.begin(),
                                     lhs_stl_container.end(),
                                     *it) == lhs_stl_container.end()) {
          if (printed_header2) {
            *os << ", ";
          } else {
            *os << (printed_header ? ",\nand" : "which")
                << " doesn't have these expected elements: ";
            printed_header2 = true;
          }
          UniversalPrint(*it, os);
        }
      }
    }

    return false;
  }

 private:
  const StlContainer expected_;
};

// A comparator functor that uses the < operator to compare two values.
struct LessComparator {
  template <typename T, typename U>
  bool operator()(const T& lhs, const U& rhs) const {
    return lhs < rhs;
  }
};

// Implements WhenSortedBy(comparator, container_matcher).
template <typename Comparator, typename ContainerMatcher>
class WhenSortedByMatcher {
 public:
  WhenSortedByMatcher(const Comparator& comparator,
                      const ContainerMatcher& matcher)
      : comparator_(comparator), matcher_(matcher) {}

  template <typename LhsContainer>
  operator Matcher<LhsContainer>() const {
    return MakeMatcher(new Impl<LhsContainer>(comparator_, matcher_));
  }

  template <typename LhsContainer>
  class Impl : public MatcherInterface<LhsContainer> {
   public:
    typedef internal::StlContainerView<GTEST_REMOVE_REFERENCE_AND_CONST_(
        LhsContainer)>
        LhsView;
    typedef typename LhsView::type LhsStlContainer;
    typedef typename LhsView::const_reference LhsStlContainerReference;
    // Transforms std::pair<const Key, Value> into std::pair<Key, Value>
    // so that we can match associative containers.
    typedef
        typename RemoveConstFromKey<typename LhsStlContainer::value_type>::type
            LhsValue;

    Impl(const Comparator& comparator, const ContainerMatcher& matcher)
        : comparator_(comparator), matcher_(matcher) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "(when sorted) ";
      matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "(when sorted) ";
      matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(LhsContainer lhs,
                         MatchResultListener* listener) const override {
      LhsStlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
      ::std::vector<LhsValue> sorted_container(lhs_stl_container.begin(),
                                               lhs_stl_container.end());
      ::std::sort(sorted_container.begin(), sorted_container.end(),
                  comparator_);

      if (!listener->IsInterested()) {
        // If the listener is not interested, we do not need to
        // construct the inner explanation.
        return matcher_.Matches(sorted_container);
      }

      *listener << "which is ";
      UniversalPrint(sorted_container, listener->stream());
      *listener << " when sorted";

      StringMatchResultListener inner_listener;
      const bool match =
          matcher_.MatchAndExplain(sorted_container, &inner_listener);
      PrintIfNotEmpty(inner_listener.str(), listener->stream());
      return match;
    }

   private:
    const Comparator comparator_;
    const Matcher<const ::std::vector<LhsValue>&> matcher_;

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
  };

 private:
  const Comparator comparator_;
  const ContainerMatcher matcher_;
};

// Implements Pointwise(tuple_matcher, rhs_container).  tuple_matcher
// must be able to be safely cast to Matcher<std::tuple<const T1&, const
// T2&> >, where T1 and T2 are the types of elements in the LHS
// container and the RHS container respectively.
template <typename TupleMatcher, typename RhsContainer>
class PointwiseMatcher {
  static_assert(
      !IsHashTable<GTEST_REMOVE_REFERENCE_AND_CONST_(RhsContainer)>::value,
      "use UnorderedPointwise with hash tables");

 public:
  typedef internal::StlContainerView<RhsContainer> RhsView;
  typedef typename RhsView::type RhsStlContainer;
  typedef typename RhsStlContainer::value_type RhsValue;

  static_assert(!std::is_const<RhsContainer>::value,
                "RhsContainer type must not be const");
  static_assert(!std::is_reference<RhsContainer>::value,
                "RhsContainer type must not be a reference");

  // Like ContainerEq, we make a copy of rhs in case the elements in
  // it are modified after this matcher is created.
  PointwiseMatcher(const TupleMatcher& tuple_matcher, const RhsContainer& rhs)
      : tuple_matcher_(tuple_matcher), rhs_(RhsView::Copy(rhs)) {}

  template <typename LhsContainer>
  operator Matcher<LhsContainer>() const {
    static_assert(
        !IsHashTable<GTEST_REMOVE_REFERENCE_AND_CONST_(LhsContainer)>::value,
        "use UnorderedPointwise with hash tables");

    return Matcher<LhsContainer>(
        new Impl<const LhsContainer&>(tuple_matcher_, rhs_));
  }

  template <typename LhsContainer>
  class Impl : public MatcherInterface<LhsContainer> {
   public:
    typedef internal::StlContainerView<GTEST_REMOVE_REFERENCE_AND_CONST_(
        LhsContainer)>
        LhsView;
    typedef typename LhsView::type LhsStlContainer;
    typedef typename LhsView::const_reference LhsStlContainerReference;
    typedef typename LhsStlContainer::value_type LhsValue;
    // We pass the LHS value and the RHS value to the inner matcher by
    // reference, as they may be expensive to copy.  We must use tuple
    // instead of pair here, as a pair cannot hold references (C++ 98,
    // 20.2.2 [lib.pairs]).
    typedef ::std::tuple<const LhsValue&, const RhsValue&> InnerMatcherArg;

    Impl(const TupleMatcher& tuple_matcher, const RhsStlContainer& rhs)
        // mono_tuple_matcher_ holds a monomorphic version of the tuple matcher.
        : mono_tuple_matcher_(SafeMatcherCast<InnerMatcherArg>(tuple_matcher)),
          rhs_(rhs) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "contains " << rhs_.size()
          << " values, where each value and its corresponding value in ";
      UniversalPrinter<RhsStlContainer>::Print(rhs_, os);
      *os << " ";
      mono_tuple_matcher_.DescribeTo(os);
    }
    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "doesn't contain exactly " << rhs_.size()
          << " values, or contains a value x at some index i"
          << " where x and the i-th value of ";
      UniversalPrint(rhs_, os);
      *os << " ";
      mono_tuple_matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(LhsContainer lhs,
                         MatchResultListener* listener) const override {
      LhsStlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
      const size_t actual_size = lhs_stl_container.size();
      if (actual_size != rhs_.size()) {
        *listener << "which contains " << actual_size << " values";
        return false;
      }

      auto left = lhs_stl_container.begin();
      auto right = rhs_.begin();
      for (size_t i = 0; i != actual_size; ++i, ++left, ++right) {
        if (listener->IsInterested()) {
          StringMatchResultListener inner_listener;
          // Create InnerMatcherArg as a temporarily object to avoid it outlives
          // *left and *right. Dereference or the conversion to `const T&` may
          // return temp objects, e.g. for vector<bool>.
          if (!mono_tuple_matcher_.MatchAndExplain(
                  InnerMatcherArg(ImplicitCast_<const LhsValue&>(*left),
                                  ImplicitCast_<const RhsValue&>(*right)),
                  &inner_listener)) {
            *listener << "where the value pair (";
            UniversalPrint(*left, listener->stream());
            *listener << ", ";
            UniversalPrint(*right, listener->stream());
            *listener << ") at index #" << i << " don't match";
            PrintIfNotEmpty(inner_listener.str(), listener->stream());
            return false;
          }
        } else {
          if (!mono_tuple_matcher_.Matches(
                  InnerMatcherArg(ImplicitCast_<const LhsValue&>(*left),
                                  ImplicitCast_<const RhsValue&>(*right))))
            return false;
        }
      }

      return true;
    }

   private:
    const Matcher<InnerMatcherArg> mono_tuple_matcher_;
    const RhsStlContainer rhs_;
  };

 private:
  const TupleMatcher tuple_matcher_;
  const RhsStlContainer rhs_;
};

// Holds the logic common to ContainsMatcherImpl and EachMatcherImpl.
template <typename Container>
class QuantifierMatcherImpl : public MatcherInterface<Container> {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
  typedef StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  template <typename InnerMatcher>
  explicit QuantifierMatcherImpl(InnerMatcher inner_matcher)
      : inner_matcher_(
            testing::SafeMatcherCast<const Element&>(inner_matcher)) {}

  // Checks whether:
  // * All elements in the container match, if all_elements_should_match.
  // * Any element in the container matches, if !all_elements_should_match.
  bool MatchAndExplainImpl(bool all_elements_should_match, Container container,
                           MatchResultListener* listener) const {
    StlContainerReference stl_container = View::ConstReference(container);
    size_t i = 0;
    for (auto it = stl_container.begin(); it != stl_container.end();
         ++it, ++i) {
      StringMatchResultListener inner_listener;
      const bool matches = inner_matcher_.MatchAndExplain(*it, &inner_listener);

      if (matches != all_elements_should_match) {
        *listener << "whose element #" << i
                  << (matches ? " matches" : " doesn't match");
        PrintIfNotEmpty(inner_listener.str(), listener->stream());
        return !all_elements_should_match;
      }
    }
    return all_elements_should_match;
  }

  bool MatchAndExplainImpl(const Matcher<size_t>& count_matcher,
                           Container container,
                           MatchResultListener* listener) const {
    StlContainerReference stl_container = View::ConstReference(container);
    size_t i = 0;
    std::vector<size_t> match_elements;
    for (auto it = stl_container.begin(); it != stl_container.end();
         ++it, ++i) {
      StringMatchResultListener inner_listener;
      const bool matches = inner_matcher_.MatchAndExplain(*it, &inner_listener);
      if (matches) {
        match_elements.push_back(i);
      }
    }
    if (listener->IsInterested()) {
      if (match_elements.empty()) {
        *listener << "has no element that matches";
      } else if (match_elements.size() == 1) {
        *listener << "whose element #" << match_elements[0] << " matches";
      } else {
        *listener << "whose elements (";
        std::string sep = "";
        for (size_t e : match_elements) {
          *listener << sep << e;
          sep = ", ";
        }
        *listener << ") match";
      }
    }
    StringMatchResultListener count_listener;
    if (count_matcher.MatchAndExplain(match_elements.size(), &count_listener)) {
      *listener << " and whose match quantity of " << match_elements.size()
                << " matches";
      PrintIfNotEmpty(count_listener.str(), listener->stream());
      return true;
    } else {
      if (match_elements.empty()) {
        *listener << " and";
      } else {
        *listener << " but";
      }
      *listener << " whose match quantity of " << match_elements.size()
                << " does not match";
      PrintIfNotEmpty(count_listener.str(), listener->stream());
      return false;
    }
  }

 protected:
  const Matcher<const Element&> inner_matcher_;
};

// Implements Contains(element_matcher) for the given argument type Container.
// Symmetric to EachMatcherImpl.
template <typename Container>
class ContainsMatcherImpl : public QuantifierMatcherImpl<Container> {
 public:
  template <typename InnerMatcher>
  explicit ContainsMatcherImpl(InnerMatcher inner_matcher)
      : QuantifierMatcherImpl<Container>(inner_matcher) {}

  // Describes what this matcher does.
  void DescribeTo(::std::ostream* os) const override {
    *os << "contains at least one element that ";
    this->inner_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "doesn't contain any element that ";
    this->inner_matcher_.DescribeTo(os);
  }

  bool MatchAndExplain(Container container,
                       MatchResultListener* listener) const override {
    return this->MatchAndExplainImpl(false, container, listener);
  }
};

// Implements Each(element_matcher) for the given argument type Container.
// Symmetric to ContainsMatcherImpl.
template <typename Container>
class EachMatcherImpl : public QuantifierMatcherImpl<Container> {
 public:
  template <typename InnerMatcher>
  explicit EachMatcherImpl(InnerMatcher inner_matcher)
      : QuantifierMatcherImpl<Container>(inner_matcher) {}

  // Describes what this matcher does.
  void DescribeTo(::std::ostream* os) const override {
    *os << "only contains elements that ";
    this->inner_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "contains some element that ";
    this->inner_matcher_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(Container container,
                       MatchResultListener* listener) const override {
    return this->MatchAndExplainImpl(true, container, listener);
  }
};

// Implements Contains(element_matcher).Times(n) for the given argument type
// Container.
template <typename Container>
class ContainsTimesMatcherImpl : public QuantifierMatcherImpl<Container> {
 public:
  template <typename InnerMatcher>
  explicit ContainsTimesMatcherImpl(InnerMatcher inner_matcher,
                                    Matcher<size_t> count_matcher)
      : QuantifierMatcherImpl<Container>(inner_matcher),
        count_matcher_(std::move(count_matcher)) {}

  void DescribeTo(::std::ostream* os) const override {
    *os << "quantity of elements that match ";
    this->inner_matcher_.DescribeTo(os);
    *os << " ";
    count_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "quantity of elements that match ";
    this->inner_matcher_.DescribeTo(os);
    *os << " ";
    count_matcher_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(Container container,
                       MatchResultListener* listener) const override {
    return this->MatchAndExplainImpl(count_matcher_, container, listener);
  }

 private:
  const Matcher<size_t> count_matcher_;
};

// Implements polymorphic Contains(element_matcher).Times(n).
template <typename M>
class ContainsTimesMatcher {
 public:
  explicit ContainsTimesMatcher(M m, Matcher<size_t> count_matcher)
      : inner_matcher_(m), count_matcher_(std::move(count_matcher)) {}

  template <typename Container>
  operator Matcher<Container>() const {  // NOLINT
    return Matcher<Container>(new ContainsTimesMatcherImpl<const Container&>(
        inner_matcher_, count_matcher_));
  }

 private:
  const M inner_matcher_;
  const Matcher<size_t> count_matcher_;
};

// Implements polymorphic Contains(element_matcher).
template <typename M>
class ContainsMatcher {
 public:
  explicit ContainsMatcher(M m) : inner_matcher_(m) {}

  template <typename Container>
  operator Matcher<Container>() const {  // NOLINT
    return Matcher<Container>(
        new ContainsMatcherImpl<const Container&>(inner_matcher_));
  }

  ContainsTimesMatcher<M> Times(Matcher<size_t> count_matcher) const {
    return ContainsTimesMatcher<M>(inner_matcher_, std::move(count_matcher));
  }

 private:
  const M inner_matcher_;
};

// Implements polymorphic Each(element_matcher).
template <typename M>
class EachMatcher {
 public:
  explicit EachMatcher(M m) : inner_matcher_(m) {}

  template <typename Container>
  operator Matcher<Container>() const {  // NOLINT
    return Matcher<Container>(
        new EachMatcherImpl<const Container&>(inner_matcher_));
  }

 private:
  const M inner_matcher_;
};

// Use go/ranked-overloads for dispatching.
struct Rank0 {};
struct Rank1 : Rank0 {};

namespace pair_getters {
using std::get;
template <typename T>
auto First(T& x, Rank0) -> decltype(get<0>(x)) {  // NOLINT
  return get<0>(x);
}
template <typename T>
auto First(T& x, Rank1) -> decltype((x.first)) {  // NOLINT
  return x.first;
}

template <typename T>
auto Second(T& x, Rank0) -> decltype(get<1>(x)) {  // NOLINT
  return get<1>(x);
}
template <typename T>
auto Second(T& x, Rank1) -> decltype((x.second)) {  // NOLINT
  return x.second;
}
}  // namespace pair_getters

// Implements Key(inner_matcher) for the given argument pair type.
// Key(inner_matcher) matches an std::pair whose 'first' field matches
// inner_matcher.  For example, Contains(Key(Ge(5))) can be used to match an
// std::map that contains at least one element whose key is >= 5.
template <typename PairType>
class KeyMatcherImpl : public MatcherInterface<PairType> {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(PairType) RawPairType;
  typedef typename RawPairType::first_type KeyType;

  template <typename InnerMatcher>
  explicit KeyMatcherImpl(InnerMatcher inner_matcher)
      : inner_matcher_(
            testing::SafeMatcherCast<const KeyType&>(inner_matcher)) {}

  // Returns true if and only if 'key_value.first' (the key) matches the inner
  // matcher.
  bool MatchAndExplain(PairType key_value,
                       MatchResultListener* listener) const override {
    StringMatchResultListener inner_listener;
    const bool match = inner_matcher_.MatchAndExplain(
        pair_getters::First(key_value, Rank1()), &inner_listener);
    const std::string explanation = inner_listener.str();
    if (!explanation.empty()) {
      *listener << "whose first field is a value " << explanation;
    }
    return match;
  }

  // Describes what this matcher does.
  void DescribeTo(::std::ostream* os) const override {
    *os << "has a key that ";
    inner_matcher_.DescribeTo(os);
  }

  // Describes what the negation of this matcher does.
  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "doesn't have a key that ";
    inner_matcher_.DescribeTo(os);
  }

 private:
  const Matcher<const KeyType&> inner_matcher_;
};

// Implements polymorphic Key(matcher_for_key).
template <typename M>
class KeyMatcher {
 public:
  explicit KeyMatcher(M m) : matcher_for_key_(m) {}

  template <typename PairType>
  operator Matcher<PairType>() const {
    return Matcher<PairType>(
        new KeyMatcherImpl<const PairType&>(matcher_for_key_));
  }

 private:
  const M matcher_for_key_;
};

// Implements polymorphic Address(matcher_for_address).
template <typename InnerMatcher>
class AddressMatcher {
 public:
  explicit AddressMatcher(InnerMatcher m) : matcher_(m) {}

  template <typename Type>
  operator Matcher<Type>() const {  // NOLINT
    return Matcher<Type>(new Impl<const Type&>(matcher_));
  }

 private:
  // The monomorphic implementation that works for a particular object type.
  template <typename Type>
  class Impl : public MatcherInterface<Type> {
   public:
    using Address = const GTEST_REMOVE_REFERENCE_AND_CONST_(Type) *;
    explicit Impl(const InnerMatcher& matcher)
        : matcher_(MatcherCast<Address>(matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "has address that ";
      matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "does not have address that ";
      matcher_.DescribeTo(os);
    }

    bool MatchAndExplain(Type object,
                         MatchResultListener* listener) const override {
      *listener << "which has address ";
      Address address = std::addressof(object);
      return MatchPrintAndExplain(address, matcher_, listener);
    }

   private:
    const Matcher<Address> matcher_;
  };
  const InnerMatcher matcher_;
};

// Implements Pair(first_matcher, second_matcher) for the given argument pair
// type with its two matchers. See Pair() function below.
template <typename PairType>
class PairMatcherImpl : public MatcherInterface<PairType> {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(PairType) RawPairType;
  typedef typename RawPairType::first_type FirstType;
  typedef typename RawPairType::second_type SecondType;

  template <typename FirstMatcher, typename SecondMatcher>
  PairMatcherImpl(FirstMatcher first_matcher, SecondMatcher second_matcher)
      : first_matcher_(
            testing::SafeMatcherCast<const FirstType&>(first_matcher)),
        second_matcher_(
            testing::SafeMatcherCast<const SecondType&>(second_matcher)) {}

  // Describes what this matcher does.
  void DescribeTo(::std::ostream* os) const override {
    *os << "has a first field that ";
    first_matcher_.DescribeTo(os);
    *os << ", and has a second field that ";
    second_matcher_.DescribeTo(os);
  }

  // Describes what the negation of this matcher does.
  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "has a first field that ";
    first_matcher_.DescribeNegationTo(os);
    *os << ", or has a second field that ";
    second_matcher_.DescribeNegationTo(os);
  }

  // Returns true if and only if 'a_pair.first' matches first_matcher and
  // 'a_pair.second' matches second_matcher.
  bool MatchAndExplain(PairType a_pair,
                       MatchResultListener* listener) const override {
    if (!listener->IsInterested()) {
      // If the listener is not interested, we don't need to construct the
      // explanation.
      return first_matcher_.Matches(pair_getters::First(a_pair, Rank1())) &&
             second_matcher_.Matches(pair_getters::Second(a_pair, Rank1()));
    }
    StringMatchResultListener first_inner_listener;
    if (!first_matcher_.MatchAndExplain(pair_getters::First(a_pair, Rank1()),
                                        &first_inner_listener)) {
      *listener << "whose first field does not match";
      PrintIfNotEmpty(first_inner_listener.str(), listener->stream());
      return false;
    }
    StringMatchResultListener second_inner_listener;
    if (!second_matcher_.MatchAndExplain(pair_getters::Second(a_pair, Rank1()),
                                         &second_inner_listener)) {
      *listener << "whose second field does not match";
      PrintIfNotEmpty(second_inner_listener.str(), listener->stream());
      return false;
    }
    ExplainSuccess(first_inner_listener.str(), second_inner_listener.str(),
                   listener);
    return true;
  }

 private:
  void ExplainSuccess(const std::string& first_explanation,
                      const std::string& second_explanation,
                      MatchResultListener* listener) const {
    *listener << "whose both fields match";
    if (!first_explanation.empty()) {
      *listener << ", where the first field is a value " << first_explanation;
    }
    if (!second_explanation.empty()) {
      *listener << ", ";
      if (!first_explanation.empty()) {
        *listener << "and ";
      } else {
        *listener << "where ";
      }
      *listener << "the second field is a value " << second_explanation;
    }
  }

  const Matcher<const FirstType&> first_matcher_;
  const Matcher<const SecondType&> second_matcher_;
};

// Implements polymorphic Pair(first_matcher, second_matcher).
template <typename FirstMatcher, typename SecondMatcher>
class PairMatcher {
 public:
  PairMatcher(FirstMatcher first_matcher, SecondMatcher second_matcher)
      : first_matcher_(first_matcher), second_matcher_(second_matcher) {}

  template <typename PairType>
  operator Matcher<PairType>() const {
    return Matcher<PairType>(
        new PairMatcherImpl<const PairType&>(first_matcher_, second_matcher_));
  }

 private:
  const FirstMatcher first_matcher_;
  const SecondMatcher second_matcher_;
};

template <typename T, size_t... I>
auto UnpackStructImpl(const T& t, std::index_sequence<I...>,
                      int) -> decltype(std::tie(get<I>(t)...)) {
  static_assert(std::tuple_size<T>::value == sizeof...(I),
                "Number of arguments doesn't match the number of fields.");
  return std::tie(get<I>(t)...);
}

#if defined(__cpp_structured_bindings) && __cpp_structured_bindings >= 201606
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<1>, char) {
  const auto& [a] = t;
  return std::tie(a);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<2>, char) {
  const auto& [a, b] = t;
  return std::tie(a, b);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<3>, char) {
  const auto& [a, b, c] = t;
  return std::tie(a, b, c);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<4>, char) {
  const auto& [a, b, c, d] = t;
  return std::tie(a, b, c, d);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<5>, char) {
  const auto& [a, b, c, d, e] = t;
  return std::tie(a, b, c, d, e);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<6>, char) {
  const auto& [a, b, c, d, e, f] = t;
  return std::tie(a, b, c, d, e, f);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<7>, char) {
  const auto& [a, b, c, d, e, f, g] = t;
  return std::tie(a, b, c, d, e, f, g);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<8>, char) {
  const auto& [a, b, c, d, e, f, g, h] = t;
  return std::tie(a, b, c, d, e, f, g, h);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<9>, char) {
  const auto& [a, b, c, d, e, f, g, h, i] = t;
  return std::tie(a, b, c, d, e, f, g, h, i);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<10>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<11>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<12>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<13>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<14>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<15>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<16>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<17>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<18>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r);
}
template <typename T>
auto UnpackStructImpl(const T& t, std::make_index_sequence<19>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s);
}
#endif  // defined(__cpp_structured_bindings)

template <size_t I, typename T>
auto UnpackStruct(const T& t)
    -> decltype((UnpackStructImpl)(t, std::make_index_sequence<I>{}, 0)) {
  return (UnpackStructImpl)(t, std::make_index_sequence<I>{}, 0);
}

// Helper function to do comma folding in C++11.
// The array ensures left-to-right order of evaluation.
// Usage: VariadicExpand({expr...});
template <typename T, size_t N>
void VariadicExpand(const T (&)[N]) {}

template <typename Struct, typename StructSize>
class FieldsAreMatcherImpl;

template <typename Struct, size_t... I>
class FieldsAreMatcherImpl<Struct, std::index_sequence<I...>>
    : public MatcherInterface<Struct> {
  using UnpackedType =
      decltype(UnpackStruct<sizeof...(I)>(std::declval<const Struct&>()));
  using MatchersType = std::tuple<
      Matcher<const typename std::tuple_element<I, UnpackedType>::type&>...>;

 public:
  template <typename Inner>
  explicit FieldsAreMatcherImpl(const Inner& matchers)
      : matchers_(testing::SafeMatcherCast<
                  const typename std::tuple_element<I, UnpackedType>::type&>(
            std::get<I>(matchers))...) {}

  void DescribeTo(::std::ostream* os) const override {
    const char* separator = "";
    VariadicExpand(
        {(*os << separator << "has field #" << I << " that ",
          std::get<I>(matchers_).DescribeTo(os), separator = ", and ")...});
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    const char* separator = "";
    VariadicExpand({(*os << separator << "has field #" << I << " that ",
                     std::get<I>(matchers_).DescribeNegationTo(os),
                     separator = ", or ")...});
  }

  bool MatchAndExplain(Struct t, MatchResultListener* listener) const override {
    return MatchInternal((UnpackStruct<sizeof...(I)>)(t), listener);
  }

 private:
  bool MatchInternal(UnpackedType tuple, MatchResultListener* listener) const {
    if (!listener->IsInterested()) {
      // If the listener is not interested, we don't need to construct the
      // explanation.
      bool good = true;
      VariadicExpand({good = good && std::get<I>(matchers_).Matches(
                                         std::get<I>(tuple))...});
      return good;
    }

    size_t failed_pos = ~size_t{};

    std::vector<StringMatchResultListener> inner_listener(sizeof...(I));

    VariadicExpand(
        {failed_pos == ~size_t{} && !std::get<I>(matchers_).MatchAndExplain(
                                        std::get<I>(tuple), &inner_listener[I])
             ? failed_pos = I
             : 0 ...});
    if (failed_pos != ~size_t{}) {
      *listener << "whose field #" << failed_pos << " does not match";
      PrintIfNotEmpty(inner_listener[failed_pos].str(), listener->stream());
      return false;
    }

    *listener << "whose all elements match";
    const char* separator = ", where";
    for (size_t index = 0; index < sizeof...(I); ++index) {
      const std::string str = inner_listener[index].str();
      if (!str.empty()) {
        *listener << separator << " field #" << index << " is a value " << str;
        separator = ", and";
      }
    }

    return true;
  }

  MatchersType matchers_;
};

template <typename... Inner>
class FieldsAreMatcher {
 public:
  explicit FieldsAreMatcher(Inner... inner) : matchers_(std::move(inner)...) {}

  template <typename Struct>
  operator Matcher<Struct>() const {  // NOLINT
    return Matcher<Struct>(
        new FieldsAreMatcherImpl<const Struct&,
                                 std::index_sequence_for<Inner...>>(matchers_));
  }

 private:
  std::tuple<Inner...> matchers_;
};

// Implements ElementsAre() and ElementsAreArray().
template <typename Container>
class ElementsAreMatcherImpl : public MatcherInterface<Container> {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
  typedef internal::StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  // Constructs the matcher from a sequence of element values or
  // element matchers.
  template <typename InputIter>
  ElementsAreMatcherImpl(InputIter first, InputIter last) {
    while (first != last) {
      matchers_.push_back(MatcherCast<const Element&>(*first++));
    }
  }

  // Describes what this matcher does.
  void DescribeTo(::std::ostream* os) const override {
    if (count() == 0) {
      *os << "is empty";
    } else if (count() == 1) {
      *os << "has 1 element that ";
      matchers_[0].DescribeTo(os);
    } else {
      *os << "has " << Elements(count()) << " where\n";
      for (size_t i = 0; i != count(); ++i) {
        *os << "element #" << i << " ";
        matchers_[i].DescribeTo(os);
        if (i + 1 < count()) {
          *os << ",\n";
        }
      }
    }
  }

  // Describes what the negation of this matcher does.
  void DescribeNegationTo(::std::ostream* os) const override {
    if (count() == 0) {
      *os << "isn't empty";
      return;
    }

    *os << "doesn't have " << Elements(count()) << ", or\n";
    for (size_t i = 0; i != count(); ++i) {
      *os << "element #" << i << " ";
      matchers_[i].DescribeNegationTo(os);
      if (i + 1 < count()) {
        *os << ", or\n";
      }
    }
  }

  bool MatchAndExplain(Container container,
                       MatchResultListener* listener) const override {
    // To work with stream-like "containers", we must only walk
    // through the elements in one pass.

    const bool listener_interested = listener->IsInterested();

    // explanations[i] is the explanation of the element at index i.
    ::std::vector<std::string> explanations(count());
    StlContainerReference stl_container = View::ConstReference(container);
    auto it = stl_container.begin();
    size_t exam_pos = 0;
    bool mismatch_found = false;  // Have we found a mismatched element yet?

    // Go through the elements and matchers in pairs, until we reach
    // the end of either the elements or the matchers, or until we find a
    // mismatch.
    for (; it != stl_container.end() && exam_pos != count(); ++it, ++exam_pos) {
      bool match;  // Does the current element match the current matcher?
      if (listener_interested) {
        StringMatchResultListener s;
        match = matchers_[exam_pos].MatchAndExplain(*it, &s);
        explanations[exam_pos] = s.str();
      } else {
        match = matchers_[exam_pos].Matches(*it);
      }

      if (!match) {
        mismatch_found = true;
        break;
      }
    }
    // If mismatch_found is true, 'exam_pos' is the index of the mismatch.

    // Find how many elements the actual container has.  We avoid
    // calling size() s.t. this code works for stream-like "containers"
    // that don't define size().
    size_t actual_count = exam_pos;
    for (; it != stl_container.end(); ++it) {
      ++actual_count;
    }

    if (actual_count != count()) {
      // The element count doesn't match.  If the container is empty,
      // there's no need to explain anything as Google Mock already
      // prints the empty container.  Otherwise we just need to show
      // how many elements there actually are.
      if (listener_interested && (actual_count != 0)) {
        *listener << "which has " << Elements(actual_count);
      }
      return false;
    }

    if (mismatch_found) {
      // The element count matches, but the exam_pos-th element doesn't match.
      if (listener_interested) {
        *listener << "whose element #" << exam_pos << " doesn't match";
        PrintIfNotEmpty(explanations[exam_pos], listener->stream());
      }
      return false;
    }

    // Every element matches its expectation.  We need to explain why
    // (the obvious ones can be skipped).
    if (listener_interested) {
      bool reason_printed = false;
      for (size_t i = 0; i != count(); ++i) {
        const std::string& s = explanations[i];
        if (!s.empty()) {
          if (reason_printed) {
            *listener << ",\nand ";
          }
          *listener << "whose element #" << i << " matches, " << s;
          reason_printed = true;
        }
      }
    }
    return true;
  }

 private:
  static Message Elements(size_t count) {
    return Message() << count << (count == 1 ? " element" : " elements");
  }

  size_t count() const { return matchers_.size(); }

  ::std::vector<Matcher<const Element&>> matchers_;
};

// Connectivity matrix of (elements X matchers), in element-major order.
// Initially, there are no edges.
// Use NextGraph() to iterate over all possible edge configurations.
// Use Randomize() to generate a random edge configuration.
class GTEST_API_ MatchMatrix {
 public:
  MatchMatrix(size_t num_elements, size_t num_matchers)
      : num_elements_(num_elements),
        num_matchers_(num_matchers),
        matched_(num_elements_ * num_matchers_, 0) {}

  size_t LhsSize() const { return num_elements_; }
  size_t RhsSize() const { return num_matchers_; }
  bool HasEdge(size_t ilhs, size_t irhs) const {
    return matched_[SpaceIndex(ilhs, irhs)] == 1;
  }
  void SetEdge(size_t ilhs, size_t irhs, bool b) {
    matched_[SpaceIndex(ilhs, irhs)] = b ? 1 : 0;
  }

  // Treating the connectivity matrix as a (LhsSize()*RhsSize())-bit number,
  // adds 1 to that number; returns false if incrementing the graph left it
  // empty.
  bool NextGraph();

  void Randomize();

  std::string DebugString() const;

 private:
  size_t SpaceIndex(size_t ilhs, size_t irhs) const {
    return ilhs * num_matchers_ + irhs;
  }

  size_t num_elements_;
  size_t num_matchers_;

  // Each element is a char interpreted as bool. They are stored as a
  // flattened array in lhs-major order, use 'SpaceIndex()' to translate
  // a (ilhs, irhs) matrix coordinate into an offset.
  ::std::vector<char> matched_;
};

typedef ::std::pair<size_t, size_t> ElementMatcherPair;
typedef ::std::vector<ElementMatcherPair> ElementMatcherPairs;

// Returns a maximum bipartite matching for the specified graph 'g'.
// The matching is represented as a vector of {element, matcher} pairs.
GTEST_API_ ElementMatcherPairs FindMaxBipartiteMatching(const MatchMatrix& g);

struct UnorderedMatcherRequire {
  enum Flags {
    Superset = 1 << 0,
    Subset = 1 << 1,
    ExactMatch = Superset | Subset,
  };
};

// Untyped base class for implementing UnorderedElementsAre.  By
// putting logic that's not specific to the element type here, we
// reduce binary bloat and increase compilation speed.
class GTEST_API_ UnorderedElementsAreMatcherImplBase {
 protected:
  explicit UnorderedElementsAreMatcherImplBase(
      UnorderedMatcherRequire::Flags matcher_flags)
      : match_flags_(matcher_flags) {}

  // A vector of matcher describers, one for each element matcher.
  // Does not own the describers (and thus can be used only when the
  // element matchers are alive).
  typedef ::std::vector<const MatcherDescriberInterface*> MatcherDescriberVec;

  // Describes this UnorderedElementsAre matcher.
  void DescribeToImpl(::std::ostream* os) const;

  // Describes the negation of this UnorderedElementsAre matcher.
  void DescribeNegationToImpl(::std::ostream* os) const;

  bool VerifyMatchMatrix(const ::std::vector<std::string>& element_printouts,
                         const MatchMatrix& matrix,
                         MatchResultListener* listener) const;

  bool FindPairing(const MatchMatrix& matrix,
                   MatchResultListener* listener) const;

  MatcherDescriberVec& matcher_describers() { return matcher_describers_; }

  static Message Elements(size_t n) {
    return Message() << n << " element" << (n == 1 ? "" : "s");
  }

  UnorderedMatcherRequire::Flags match_flags() const { return match_flags_; }

 private:
  UnorderedMatcherRequire::Flags match_flags_;
  MatcherDescriberVec matcher_describers_;
};

// Implements UnorderedElementsAre, UnorderedElementsAreArray, IsSubsetOf, and
// IsSupersetOf.
template <typename Container>
class UnorderedElementsAreMatcherImpl
    : public MatcherInterface<Container>,
      public UnorderedElementsAreMatcherImplBase {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
  typedef internal::StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  template <typename InputIter>
  UnorderedElementsAreMatcherImpl(UnorderedMatcherRequire::Flags matcher_flags,
                                  InputIter first, InputIter last)
      : UnorderedElementsAreMatcherImplBase(matcher_flags) {
    for (; first != last; ++first) {
      matchers_.push_back(MatcherCast<const Element&>(*first));
    }
    for (const auto& m : matchers_) {
      matcher_describers().push_back(m.GetDescriber());
    }
  }

  // Describes what this matcher does.
  void DescribeTo(::std::ostream* os) const override {
    return UnorderedElementsAreMatcherImplBase::DescribeToImpl(os);
  }

  // Describes what the negation of this matcher does.
  void DescribeNegationTo(::std::ostream* os) const override {
    return UnorderedElementsAreMatcherImplBase::DescribeNegationToImpl(os);
  }

  bool MatchAndExplain(Container container,
                       MatchResultListener* listener) const override {
    StlContainerReference stl_container = View::ConstReference(container);
    ::std::vector<std::string> element_printouts;
    MatchMatrix matrix =
        AnalyzeElements(stl_container.begin(), stl_container.end(),
                        &element_printouts, listener);

    return VerifyMatchMatrix(element_printouts, matrix, listener) &&
           FindPairing(matrix, listener);
  }

 private:
  template <typename ElementIter>
  MatchMatrix AnalyzeElements(ElementIter elem_first, ElementIter elem_last,
                              ::std::vector<std::string>* element_printouts,
                              MatchResultListener* listener) const {
    element_printouts->clear();
    ::std::vector<char> did_match;
    size_t num_elements = 0;
    DummyMatchResultListener dummy;
    for (; elem_first != elem_last; ++num_elements, ++elem_first) {
      if (listener->IsInterested()) {
        element_printouts->push_back(PrintToString(*elem_first));
      }
      for (size_t irhs = 0; irhs != matchers_.size(); ++irhs) {
        did_match.push_back(
            matchers_[irhs].MatchAndExplain(*elem_first, &dummy));
      }
    }

    MatchMatrix matrix(num_elements, matchers_.size());
    ::std::vector<char>::const_iterator did_match_iter = did_match.begin();
    for (size_t ilhs = 0; ilhs != num_elements; ++ilhs) {
      for (size_t irhs = 0; irhs != matchers_.size(); ++irhs) {
        matrix.SetEdge(ilhs, irhs, *did_match_iter++ != 0);
      }
    }
    return matrix;
  }

  ::std::vector<Matcher<const Element&>> matchers_;
};

// Functor for use in TransformTuple.
// Performs MatcherCast<Target> on an input argument of any type.
template <typename Target>
struct CastAndAppendTransform {
  template <typename Arg>
  Matcher<Target> operator()(const Arg& a) const {
    return MatcherCast<Target>(a);
  }
};

// Implements UnorderedElementsAre.
template <typename MatcherTuple>
class UnorderedElementsAreMatcher {
 public:
  explicit UnorderedElementsAreMatcher(const MatcherTuple& args)
      : matchers_(args) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type View;
    typedef typename View::value_type Element;
    typedef ::std::vector<Matcher<const Element&>> MatcherVec;
    MatcherVec matchers;
    matchers.reserve(::std::tuple_size<MatcherTuple>::value);
    TransformTupleValues(CastAndAppendTransform<const Element&>(), matchers_,
                         ::std::back_inserter(matchers));
    return Matcher<Container>(
        new UnorderedElementsAreMatcherImpl<const Container&>(
            UnorderedMatcherRequire::ExactMatch, matchers.begin(),
            matchers.end()));
  }

 private:
  const MatcherTuple matchers_;
};

// Implements ElementsAre.
template <typename MatcherTuple>
class ElementsAreMatcher {
 public:
  explicit ElementsAreMatcher(const MatcherTuple& args) : matchers_(args) {}

  template <typename Container>
  operator Matcher<Container>() const {
    static_assert(
        !IsHashTable<GTEST_REMOVE_REFERENCE_AND_CONST_(Container)>::value ||
            ::std::tuple_size<MatcherTuple>::value < 2,
        "use UnorderedElementsAre with hash tables");

    typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type View;
    typedef typename View::value_type Element;
    typedef ::std::vector<Matcher<const Element&>> MatcherVec;
    MatcherVec matchers;
    matchers.reserve(::std::tuple_size<MatcherTuple>::value);
    TransformTupleValues(CastAndAppendTransform<const Element&>(), matchers_,
                         ::std::back_inserter(matchers));
    return Matcher<Container>(new ElementsAreMatcherImpl<const Container&>(
        matchers.begin(), matchers.end()));
  }

 private:
  const MatcherTuple matchers_;
};

// Implements UnorderedElementsAreArray(), IsSubsetOf(), and IsSupersetOf().
template <typename T>
class UnorderedElementsAreArrayMatcher {
 public:
  template <typename Iter>
  UnorderedElementsAreArrayMatcher(UnorderedMatcherRequire::Flags match_flags,
                                   Iter first, Iter last)
      : match_flags_(match_flags), matchers_(first, last) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return Matcher<Container>(
        new UnorderedElementsAreMatcherImpl<const Container&>(
            match_flags_, matchers_.begin(), matchers_.end()));
  }

 private:
  UnorderedMatcherRequire::Flags match_flags_;
  ::std::vector<T> matchers_;
};

// Implements ElementsAreArray().
template <typename T>
class ElementsAreArrayMatcher {
 public:
  template <typename Iter>
  ElementsAreArrayMatcher(Iter first, Iter last) : matchers_(first, last) {}

  template <typename Container>
  operator Matcher<Container>() const {
    static_assert(
        !IsHashTable<GTEST_REMOVE_REFERENCE_AND_CONST_(Container)>::value,
        "use UnorderedElementsAreArray with hash tables");

    return Matcher<Container>(new ElementsAreMatcherImpl<const Container&>(
        matchers_.begin(), matchers_.end()));
  }

 private:
  const ::std::vector<T> matchers_;
};

// Given a 2-tuple matcher tm of type Tuple2Matcher and a value second
// of type Second, BoundSecondMatcher<Tuple2Matcher, Second>(tm,
// second) is a polymorphic matcher that matches a value x if and only if
// tm matches tuple (x, second).  Useful for implementing
// UnorderedPointwise() in terms of UnorderedElementsAreArray().
//
// BoundSecondMatcher is copyable and assignable, as we need to put
// instances of this class in a vector when implementing
// UnorderedPointwise().
template <typename Tuple2Matcher, typename Second>
class BoundSecondMatcher {
 public:
  BoundSecondMatcher(const Tuple2Matcher& tm, const Second& second)
      : tuple2_matcher_(tm), second_value_(second) {}

  BoundSecondMatcher(const BoundSecondMatcher& other) = default;

  template <typename T>
  operator Matcher<T>() const {
    return MakeMatcher(new Impl<T>(tuple2_matcher_, second_value_));
  }

  // We have to define this for UnorderedPointwise() to compile in
  // C++98 mode, as it puts BoundSecondMatcher instances in a vector,
  // which requires the elements to be assignable in C++98.  The
  // compiler cannot generate the operator= for us, as Tuple2Matcher
  // and Second may not be assignable.
  //
  // However, this should never be called, so the implementation just
  // need to assert.
  void operator=(const BoundSecondMatcher& /*rhs*/) {
    GTEST_LOG_(FATAL) << "BoundSecondMatcher should never be assigned.";
  }

 private:
  template <typename T>
  class Impl : public MatcherInterface<T> {
   public:
    typedef ::std::tuple<T, Second> ArgTuple;

    Impl(const Tuple2Matcher& tm, const Second& second)
        : mono_tuple2_matcher_(SafeMatcherCast<const ArgTuple&>(tm)),
          second_value_(second) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "and ";
      UniversalPrint(second_value_, os);
      *os << " ";
      mono_tuple2_matcher_.DescribeTo(os);
    }

    bool MatchAndExplain(T x, MatchResultListener* listener) const override {
      return mono_tuple2_matcher_.MatchAndExplain(ArgTuple(x, second_value_),
                                                  listener);
    }

   private:
    const Matcher<const ArgTuple&> mono_tuple2_matcher_;
    const Second second_value_;
  };

  const Tuple2Matcher tuple2_matcher_;
  const Second second_value_;
};

// Given a 2-tuple matcher tm and a value second,
// MatcherBindSecond(tm, second) returns a matcher that matches a
// value x if and only if tm matches tuple (x, second).  Useful for
// implementing UnorderedPointwise() in terms of UnorderedElementsAreArray().
template <typename Tuple2Matcher, typename Second>
BoundSecondMatcher<Tuple2Matcher, Second> MatcherBindSecond(
    const Tuple2Matcher& tm, const Second& second) {
  return BoundSecondMatcher<Tuple2Matcher, Second>(tm, second);
}

// Returns the description for a matcher defined using the MATCHER*()
// macro where the user-supplied description string is "", if
// 'negation' is false; otherwise returns the description of the
// negation of the matcher.  'param_values' contains a list of strings
// that are the print-out of the matcher's parameters.
GTEST_API_ std::string FormatMatcherDescription(
    bool negation, const char* matcher_name,
    const std::vector<const char*>& param_names, const Strings& param_values);

// Implements a matcher that checks the value of a optional<> type variable.
template <typename ValueMatcher>
class OptionalMatcher {
 public:
  explicit OptionalMatcher(const ValueMatcher& value_matcher)
      : value_matcher_(value_matcher) {}

  template <typename Optional>
  operator Matcher<Optional>() const {
    return Matcher<Optional>(new Impl<const Optional&>(value_matcher_));
  }

  template <typename Optional>
  class Impl : public MatcherInterface<Optional> {
   public:
    typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Optional) OptionalView;
    typedef typename OptionalView::value_type ValueType;
    explicit Impl(const ValueMatcher& value_matcher)
        : value_matcher_(MatcherCast<ValueType>(value_matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "value ";
      value_matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "value ";
      value_matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(Optional optional,
                         MatchResultListener* listener) const override {
      if (!optional) {
        *listener << "which is not engaged";
        return false;
      }
      const ValueType& value = *optional;
      StringMatchResultListener value_listener;
      const bool match = value_matcher_.MatchAndExplain(value, &value_listener);
      *listener << "whose value " << PrintToString(value)
                << (match ? " matches" : " doesn't match");
      PrintIfNotEmpty(value_listener.str(), listener->stream());
      return match;
    }

   private:
    const Matcher<ValueType> value_matcher_;
  };

 private:
  const ValueMatcher value_matcher_;
};

namespace variant_matcher {
// Overloads to allow VariantMatcher to do proper ADL lookup.
template <typename T>
void holds_alternative() {}
template <typename T>
void get() {}

// Implements a matcher that checks the value of a variant<> type variable.
template <typename T>
class VariantMatcher {
 public:
  explicit VariantMatcher(::testing::Matcher<const T&> matcher)
      : matcher_(std::move(matcher)) {}

  template <typename Variant>
  bool MatchAndExplain(const Variant& value,
                       ::testing::MatchResultListener* listener) const {
    using std::get;
    if (!listener->IsInterested()) {
      return holds_alternative<T>(value) && matcher_.Matches(get<T>(value));
    }

    if (!holds_alternative<T>(value)) {
      *listener << "whose value is not of type '" << GetTypeName() << "'";
      return false;
    }

    const T& elem = get<T>(value);
    StringMatchResultListener elem_listener;
    const bool match = matcher_.MatchAndExplain(elem, &elem_listener);
    *listener << "whose value " << PrintToString(elem)
              << (match ? " matches" : " doesn't match");
    PrintIfNotEmpty(elem_listener.str(), listener->stream());
    return match;
  }

  void DescribeTo(std::ostream* os) const {
    *os << "is a variant<> with value of type '" << GetTypeName()
        << "' and the value ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "is a variant<> with value of type other than '" << GetTypeName()
        << "' or the value ";
    matcher_.DescribeNegationTo(os);
  }

 private:
  static std::string GetTypeName() {
#if GTEST_HAS_RTTI
    GTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(
        return internal::GetTypeName<T>());
#endif
    return "the element type";
  }

  const ::testing::Matcher<const T&> matcher_;
};

}  // namespace variant_matcher

namespace any_cast_matcher {

// Overloads to allow AnyCastMatcher to do proper ADL lookup.
template <typename T>
void any_cast() {}

// Implements a matcher that any_casts the value.
template <typename T>
class AnyCastMatcher {
 public:
  explicit AnyCastMatcher(const ::testing::Matcher<const T&>& matcher)
      : matcher_(matcher) {}

  template <typename AnyType>
  bool MatchAndExplain(const AnyType& value,
                       ::testing::MatchResultListener* listener) const {
    if (!listener->IsInterested()) {
      const T* ptr = any_cast<T>(&value);
      return ptr != nullptr && matcher_.Matches(*ptr);
    }

    const T* elem = any_cast<T>(&value);
    if (elem == nullptr) {
      *listener << "whose value is not of type '" << GetTypeName() << "'";
      return false;
    }

    StringMatchResultListener elem_listener;
    const bool match = matcher_.MatchAndExplain(*elem, &elem_listener);
    *listener << "whose value " << PrintToString(*elem)
              << (match ? " matches" : " doesn't match");
    PrintIfNotEmpty(elem_listener.str(), listener->stream());
    return match;
  }

  void DescribeTo(std::ostream* os) const {
    *os << "is an 'any' type with value of type '" << GetTypeName()
        << "' and the value ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "is an 'any' type with value of type other than '" << GetTypeName()
        << "' or the value ";
    matcher_.DescribeNegationTo(os);
  }

 private:
  static std::string GetTypeName() {
#if GTEST_HAS_RTTI
    GTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(
        return internal::GetTypeName<T>());
#endif
    return "the element type";
  }

  const ::testing::Matcher<const T&> matcher_;
};

}  // namespace any_cast_matcher

// Implements the Args() matcher.
template <class ArgsTuple, size_t... k>
class ArgsMatcherImpl : public MatcherInterface<ArgsTuple> {
 public:
  using RawArgsTuple = typename std::decay<ArgsTuple>::type;
  using SelectedArgs =
      std::tuple<typename std::tuple_element<k, RawArgsTuple>::type...>;
  using MonomorphicInnerMatcher = Matcher<const SelectedArgs&>;

  template <typename InnerMatcher>
  explicit ArgsMatcherImpl(const InnerMatcher& inner_matcher)
      : inner_matcher_(SafeMatcherCast<const SelectedArgs&>(inner_matcher)) {}

  bool MatchAndExplain(ArgsTuple args,
                       MatchResultListener* listener) const override {
    // Workaround spurious C4100 on MSVC<=15.7 when k is empty.
    (void)args;
    const SelectedArgs& selected_args =
        std::forward_as_tuple(std::get<k>(args)...);
    if (!listener->IsInterested()) return inner_matcher_.Matches(selected_args);

    PrintIndices(listener->stream());
    *listener << "are " << PrintToString(selected_args);

    StringMatchResultListener inner_listener;
    const bool match =
        inner_matcher_.MatchAndExplain(selected_args, &inner_listener);
    PrintIfNotEmpty(inner_listener.str(), listener->stream());
    return match;
  }

  void DescribeTo(::std::ostream* os) const override {
    *os << "are a tuple ";
    PrintIndices(os);
    inner_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "are a tuple ";
    PrintIndices(os);
    inner_matcher_.DescribeNegationTo(os);
  }

 private:
  // Prints the indices of the selected fields.
  static void PrintIndices(::std::ostream* os) {
    *os << "whose fields (";
    const char* sep = "";
    // Workaround spurious C4189 on MSVC<=15.7 when k is empty.
    (void)sep;
    // The static_cast to void is needed to silence Clang's -Wcomma warning.
    // This pattern looks suspiciously like we may have mismatched parentheses
    // and may have been trying to use the first operation of the comma operator
    // as a member of the array, so Clang warns that we may have made a mistake.
    const char* dummy[] = {
        "", (static_cast<void>(*os << sep << "#" << k), sep = ", ")...};
    (void)dummy;
    *os << ") ";
  }

  MonomorphicInnerMatcher inner_matcher_;
};

template <class InnerMatcher, size_t... k>
class ArgsMatcher {
 public:
  explicit ArgsMatcher(InnerMatcher inner_matcher)
      : inner_matcher_(std::move(inner_matcher)) {}

  template <typename ArgsTuple>
  operator Matcher<ArgsTuple>() const {  // NOLINT
    return MakeMatcher(new ArgsMatcherImpl<ArgsTuple, k...>(inner_matcher_));
  }

 private:
  InnerMatcher inner_matcher_;
};

}  // namespace internal

// ElementsAreArray(iterator_first, iterator_last)
// ElementsAreArray(pointer, count)
// ElementsAreArray(array)
// ElementsAreArray(container)
// ElementsAreArray({ e1, e2, ..., en })
//
// The ElementsAreArray() functions are like ElementsAre(...), except
// that they are given a homogeneous sequence rather than taking each
// element as a function argument. The sequence can be specified as an
// array, a pointer and count, a vector, an initializer list, or an
// STL iterator range. In each of these cases, the underlying sequence
// can be either a sequence of values or a sequence of matchers.
//
// All forms of ElementsAreArray() make a copy of the input matcher sequence.

template <typename Iter>
inline internal::ElementsAreArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
ElementsAreArray(Iter first, Iter last) {
  typedef typename ::std::iterator_traits<Iter>::value_type T;
  return internal::ElementsAreArrayMatcher<T>(first, last);
}

template <typename T>
inline auto ElementsAreArray(const T* pointer, size_t count)
    -> decltype(ElementsAreArray(pointer, pointer + count)) {
  return ElementsAreArray(pointer, pointer + count);
}

template <typename T, size_t N>
inline auto ElementsAreArray(const T (&array)[N])
    -> decltype(ElementsAreArray(array, N)) {
  return ElementsAreArray(array, N);
}

template <typename Container>
inline auto ElementsAreArray(const Container& container)
    -> decltype(ElementsAreArray(container.begin(), container.end())) {
  return ElementsAreArray(container.begin(), container.end());
}

template <typename T>
inline auto ElementsAreArray(::std::initializer_list<T> xs)
    -> decltype(ElementsAreArray(xs.begin(), xs.end())) {
  return ElementsAreArray(xs.begin(), xs.end());
}

// UnorderedElementsAreArray(iterator_first, iterator_last)
// UnorderedElementsAreArray(pointer, count)
// UnorderedElementsAreArray(array)
// UnorderedElementsAreArray(container)
// UnorderedElementsAreArray({ e1, e2, ..., en })
//
// UnorderedElementsAreArray() verifies that a bijective mapping onto a
// collection of matchers exists.
//
// The matchers can be specified as an array, a pointer and count, a container,
// an initializer list, or an STL iterator range. In each of these cases, the
// underlying matchers can be either values or matchers.

template <typename Iter>
inline internal::UnorderedElementsAreArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
UnorderedElementsAreArray(Iter first, Iter last) {
  typedef typename ::std::iterator_traits<Iter>::value_type T;
  return internal::UnorderedElementsAreArrayMatcher<T>(
      internal::UnorderedMatcherRequire::ExactMatch, first, last);
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T> UnorderedElementsAreArray(
    const T* pointer, size_t count) {
  return UnorderedElementsAreArray(pointer, pointer + count);
}

template <typename T, size_t N>
inline internal::UnorderedElementsAreArrayMatcher<T> UnorderedElementsAreArray(
    const T (&array)[N]) {
  return UnorderedElementsAreArray(array, N);
}

template <typename Container>
inline internal::UnorderedElementsAreArrayMatcher<
    typename Container::value_type>
UnorderedElementsAreArray(const Container& container) {
  return UnorderedElementsAreArray(container.begin(), container.end());
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T> UnorderedElementsAreArray(
    ::std::initializer_list<T> xs) {
  return UnorderedElementsAreArray(xs.begin(), xs.end());
}

// _ is a matcher that matches anything of any type.
//
// This definition is fine as:
//
//   1. The C++ standard permits using the name _ in a namespace that
//      is not the global namespace or ::std.
//   2. The AnythingMatcher class has no data member or constructor,
//      so it's OK to create global variables of this type.
//   3. c-style has approved of using _ in this case.
const internal::AnythingMatcher _ = {};
// Creates a matcher that matches any value of the given type T.
template <typename T>
inline Matcher<T> A() {
  return _;
}

// Creates a matcher that matches any value of the given type T.
template <typename T>
inline Matcher<T> An() {
  return _;
}

template <typename T, typename M>
Matcher<T> internal::MatcherCastImpl<T, M>::CastImpl(
    const M& value, std::false_type /* convertible_to_matcher */,
    std::false_type /* convertible_to_T */) {
  return Eq(value);
}

// Creates a polymorphic matcher that matches any NULL pointer.
inline PolymorphicMatcher<internal::IsNullMatcher> IsNull() {
  return MakePolymorphicMatcher(internal::IsNullMatcher());
}

// Creates a polymorphic matcher that matches any non-NULL pointer.
// This is convenient as Not(NULL) doesn't compile (the compiler
// thinks that that expression is comparing a pointer with an integer).
inline PolymorphicMatcher<internal::NotNullMatcher> NotNull() {
  return MakePolymorphicMatcher(internal::NotNullMatcher());
}

// Creates a polymorphic matcher that matches any argument that
// references variable x.
template <typename T>
inline internal::RefMatcher<T&> Ref(T& x) {  // NOLINT
  return internal::RefMatcher<T&>(x);
}

// Creates a polymorphic matcher that matches any NaN floating point.
inline PolymorphicMatcher<internal::IsNanMatcher> IsNan() {
  return MakePolymorphicMatcher(internal::IsNanMatcher());
}

// Creates a matcher that matches any double argument approximately
// equal to rhs, where two NANs are considered unequal.
inline internal::FloatingEqMatcher<double> DoubleEq(double rhs) {
  return internal::FloatingEqMatcher<double>(rhs, false);
}

// Creates a matcher that matches any double argument approximately
// equal to rhs, including NaN values when rhs is NaN.
inline internal::FloatingEqMatcher<double> NanSensitiveDoubleEq(double rhs) {
  return internal::FloatingEqMatcher<double>(rhs, true);
}

// Creates a matcher that matches any double argument approximately equal to
// rhs, up to the specified max absolute error bound, where two NANs are
// considered unequal.  The max absolute error bound must be non-negative.
inline internal::FloatingEqMatcher<double> DoubleNear(double rhs,
                                                      double max_abs_error) {
  return internal::FloatingEqMatcher<double>(rhs, false, max_abs_error);
}

// Creates a matcher that matches any double argument approximately equal to
// rhs, up to the specified max absolute error bound, including NaN values when
// rhs is NaN.  The max absolute error bound must be non-negative.
inline internal::FloatingEqMatcher<double> NanSensitiveDoubleNear(
    double rhs, double max_abs_error) {
  return internal::FloatingEqMatcher<double>(rhs, true, max_abs_error);
}

// Creates a matcher that matches any float argument approximately
// equal to rhs, where two NANs are considered unequal.
inline internal::FloatingEqMatcher<float> FloatEq(float rhs) {
  return internal::FloatingEqMatcher<float>(rhs, false);
}

// Creates a matcher that matches any float argument approximately
// equal to rhs, including NaN values when rhs is NaN.
inline internal::FloatingEqMatcher<float> NanSensitiveFloatEq(float rhs) {
  return internal::FloatingEqMatcher<float>(rhs, true);
}

// Creates a matcher that matches any float argument approximately equal to
// rhs, up to the specified max absolute error bound, where two NANs are
// considered unequal.  The max absolute error bound must be non-negative.
inline internal::FloatingEqMatcher<float> FloatNear(float rhs,
                                                    float max_abs_error) {
  return internal::FloatingEqMatcher<float>(rhs, false, max_abs_error);
}

// Creates a matcher that matches any float argument approximately equal to
// rhs, up to the specified max absolute error bound, including NaN values when
// rhs is NaN.  The max absolute error bound must be non-negative.
inline internal::FloatingEqMatcher<float> NanSensitiveFloatNear(
    float rhs, float max_abs_error) {
  return internal::FloatingEqMatcher<float>(rhs, true, max_abs_error);
}

// Creates a matcher that matches a pointer (raw or smart) that points
// to a value that matches inner_matcher.
template <typename InnerMatcher>
inline internal::PointeeMatcher<InnerMatcher> Pointee(
    const InnerMatcher& inner_matcher) {
  return internal::PointeeMatcher<InnerMatcher>(inner_matcher);
}

#if GTEST_HAS_RTTI
// Creates a matcher that matches a pointer or reference that matches
// inner_matcher when dynamic_cast<To> is applied.
// The result of dynamic_cast<To> is forwarded to the inner matcher.
// If To is a pointer and the cast fails, the inner matcher will receive NULL.
// If To is a reference and the cast fails, this matcher returns false
// immediately.
template <typename To>
inline PolymorphicMatcher<internal::WhenDynamicCastToMatcher<To>>
WhenDynamicCastTo(const Matcher<To>& inner_matcher) {
  return MakePolymorphicMatcher(
      internal::WhenDynamicCastToMatcher<To>(inner_matcher));
}
#endif  // GTEST_HAS_RTTI

// Creates a matcher that matches an object whose given field matches
// 'matcher'.  For example,
//   Field(&Foo::number, Ge(5))
// matches a Foo object x if and only if x.number >= 5.
template <typename Class, typename FieldType, typename FieldMatcher>
inline PolymorphicMatcher<internal::FieldMatcher<Class, FieldType>> Field(
    FieldType Class::*field, const FieldMatcher& matcher) {
  return MakePolymorphicMatcher(internal::FieldMatcher<Class, FieldType>(
      field, MatcherCast<const FieldType&>(matcher)));
  // The call to MatcherCast() is required for supporting inner
  // matchers of compatible types.  For example, it allows
  //   Field(&Foo::bar, m)
  // to compile where bar is an int32 and m is a matcher for int64.
}

// Same as Field() but also takes the name of the field to provide better error
// messages.
template <typename Class, typename FieldType, typename FieldMatcher>
inline PolymorphicMatcher<internal::FieldMatcher<Class, FieldType>> Field(
    const std::string& field_name, FieldType Class::*field,
    const FieldMatcher& matcher) {
  return MakePolymorphicMatcher(internal::FieldMatcher<Class, FieldType>(
      field_name, field, MatcherCast<const FieldType&>(matcher)));
}

// Creates a matcher that matches an object whose given property
// matches 'matcher'.  For example,
//   Property(&Foo::str, StartsWith("hi"))
// matches a Foo object x if and only if x.str() starts with "hi".
template <typename Class, typename PropertyType, typename PropertyMatcher>
inline PolymorphicMatcher<internal::PropertyMatcher<
    Class, PropertyType, PropertyType (Class::*)() const>>
Property(PropertyType (Class::*property)() const,
         const PropertyMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::PropertyMatcher<Class, PropertyType,
                                PropertyType (Class::*)() const>(
          property, MatcherCast<const PropertyType&>(matcher)));
  // The call to MatcherCast() is required for supporting inner
  // matchers of compatible types.  For example, it allows
  //   Property(&Foo::bar, m)
  // to compile where bar() returns an int32 and m is a matcher for int64.
}

// Same as Property() above, but also takes the name of the property to provide
// better error messages.
template <typename Class, typename PropertyType, typename PropertyMatcher>
inline PolymorphicMatcher<internal::PropertyMatcher<
    Class, PropertyType, PropertyType (Class::*)() const>>
Property(const std::string& property_name,
         PropertyType (Class::*property)() const,
         const PropertyMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::PropertyMatcher<Class, PropertyType,
                                PropertyType (Class::*)() const>(
          property_name, property, MatcherCast<const PropertyType&>(matcher)));
}

// The same as above but for reference-qualified member functions.
template <typename Class, typename PropertyType, typename PropertyMatcher>
inline PolymorphicMatcher<internal::PropertyMatcher<
    Class, PropertyType, PropertyType (Class::*)() const&>>
Property(PropertyType (Class::*property)() const&,
         const PropertyMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::PropertyMatcher<Class, PropertyType,
                                PropertyType (Class::*)() const&>(
          property, MatcherCast<const PropertyType&>(matcher)));
}

// Three-argument form for reference-qualified member functions.
template <typename Class, typename PropertyType, typename PropertyMatcher>
inline PolymorphicMatcher<internal::PropertyMatcher<
    Class, PropertyType, PropertyType (Class::*)() const&>>
Property(const std::string& property_name,
         PropertyType (Class::*property)() const&,
         const PropertyMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::PropertyMatcher<Class, PropertyType,
                                PropertyType (Class::*)() const&>(
          property_name, property, MatcherCast<const PropertyType&>(matcher)));
}

// Creates a matcher that matches an object if and only if the result of
// applying a callable to x matches 'matcher'. For example,
//   ResultOf(f, StartsWith("hi"))
// matches a Foo object x if and only if f(x) starts with "hi".
// `callable` parameter can be a function, function pointer, or a functor. It is
// required to keep no state affecting the results of the calls on it and make
// no assumptions about how many calls will be made. Any state it keeps must be
// protected from the concurrent access.
template <typename Callable, typename InnerMatcher>
internal::ResultOfMatcher<Callable, InnerMatcher> ResultOf(
    Callable callable, InnerMatcher matcher) {
  return internal::ResultOfMatcher<Callable, InnerMatcher>(std::move(callable),
                                                           std::move(matcher));
}

// Same as ResultOf() above, but also takes a description of the `callable`
// result to provide better error messages.
template <typename Callable, typename InnerMatcher>
internal::ResultOfMatcher<Callable, InnerMatcher> ResultOf(
    const std::string& result_description, Callable callable,
    InnerMatcher matcher) {
  return internal::ResultOfMatcher<Callable, InnerMatcher>(
      result_description, std::move(callable), std::move(matcher));
}

// String matchers.

// Matches a string equal to str.
template <typename T = std::string>
PolymorphicMatcher<internal::StrEqualityMatcher<std::string>> StrEq(
    const internal::StringLike<T>& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::string>(std::string(str), true, true));
}

// Matches a string not equal to str.
template <typename T = std::string>
PolymorphicMatcher<internal::StrEqualityMatcher<std::string>> StrNe(
    const internal::StringLike<T>& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::string>(std::string(str), false, true));
}

// Matches a string equal to str, ignoring case.
template <typename T = std::string>
PolymorphicMatcher<internal::StrEqualityMatcher<std::string>> StrCaseEq(
    const internal::StringLike<T>& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::string>(std::string(str), true, false));
}

// Matches a string not equal to str, ignoring case.
template <typename T = std::string>
PolymorphicMatcher<internal::StrEqualityMatcher<std::string>> StrCaseNe(
    const internal::StringLike<T>& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<std::string>(
      std::string(str), false, false));
}

// Creates a matcher that matches any string, std::string, or C string
// that contains the given substring.
template <typename T = std::string>
PolymorphicMatcher<internal::HasSubstrMatcher<std::string>> HasSubstr(
    const internal::StringLike<T>& substring) {
  return MakePolymorphicMatcher(
      internal::HasSubstrMatcher<std::string>(std::string(substring)));
}

// Matches a string that starts with 'prefix' (case-sensitive).
template <typename T = std::string>
PolymorphicMatcher<internal::StartsWithMatcher<std::string>> StartsWith(
    const internal::StringLike<T>& prefix) {
  return MakePolymorphicMatcher(
      internal::StartsWithMatcher<std::string>(std::string(prefix)));
}

// Matches a string that ends with 'suffix' (case-sensitive).
template <typename T = std::string>
PolymorphicMatcher<internal::EndsWithMatcher<std::string>> EndsWith(
    const internal::StringLike<T>& suffix) {
  return MakePolymorphicMatcher(
      internal::EndsWithMatcher<std::string>(std::string(suffix)));
}

#if GTEST_HAS_STD_WSTRING
// Wide string matchers.

// Matches a string equal to str.
inline PolymorphicMatcher<internal::StrEqualityMatcher<std::wstring>> StrEq(
    const std::wstring& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::wstring>(str, true, true));
}

// Matches a string not equal to str.
inline PolymorphicMatcher<internal::StrEqualityMatcher<std::wstring>> StrNe(
    const std::wstring& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::wstring>(str, false, true));
}

// Matches a string equal to str, ignoring case.
inline PolymorphicMatcher<internal::StrEqualityMatcher<std::wstring>> StrCaseEq(
    const std::wstring& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::wstring>(str, true, false));
}

// Matches a string not equal to str, ignoring case.
inline PolymorphicMatcher<internal::StrEqualityMatcher<std::wstring>> StrCaseNe(
    const std::wstring& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::wstring>(str, false, false));
}

// Creates a matcher that matches any ::wstring, std::wstring, or C wide string
// that contains the given substring.
inline PolymorphicMatcher<internal::HasSubstrMatcher<std::wstring>> HasSubstr(
    const std::wstring& substring) {
  return MakePolymorphicMatcher(
      internal::HasSubstrMatcher<std::wstring>(substring));
}

// Matches a string that starts with 'prefix' (case-sensitive).
inline PolymorphicMatcher<internal::StartsWithMatcher<std::wstring>> StartsWith(
    const std::wstring& prefix) {
  return MakePolymorphicMatcher(
      internal::StartsWithMatcher<std::wstring>(prefix));
}

// Matches a string that ends with 'suffix' (case-sensitive).
inline PolymorphicMatcher<internal::EndsWithMatcher<std::wstring>> EndsWith(
    const std::wstring& suffix) {
  return MakePolymorphicMatcher(
      internal::EndsWithMatcher<std::wstring>(suffix));
}

#endif  // GTEST_HAS_STD_WSTRING

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field == the second field.
inline internal::Eq2Matcher Eq() { return internal::Eq2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field >= the second field.
inline internal::Ge2Matcher Ge() { return internal::Ge2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field > the second field.
inline internal::Gt2Matcher Gt() { return internal::Gt2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field <= the second field.
inline internal::Le2Matcher Le() { return internal::Le2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field < the second field.
inline internal::Lt2Matcher Lt() { return internal::Lt2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field != the second field.
inline internal::Ne2Matcher Ne() { return internal::Ne2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where
// FloatEq(first field) matches the second field.
inline internal::FloatingEq2Matcher<float> FloatEq() {
  return internal::FloatingEq2Matcher<float>();
}

// Creates a polymorphic matcher that matches a 2-tuple where
// DoubleEq(first field) matches the second field.
inline internal::FloatingEq2Matcher<double> DoubleEq() {
  return internal::FloatingEq2Matcher<double>();
}

// Creates a polymorphic matcher that matches a 2-tuple where
// FloatEq(first field) matches the second field with NaN equality.
inline internal::FloatingEq2Matcher<float> NanSensitiveFloatEq() {
  return internal::FloatingEq2Matcher<float>(true);
}

// Creates a polymorphic matcher that matches a 2-tuple where
// DoubleEq(first field) matches the second field with NaN equality.
inline internal::FloatingEq2Matcher<double> NanSensitiveDoubleEq() {
  return internal::FloatingEq2Matcher<double>(true);
}

// Creates a polymorphic matcher that matches a 2-tuple where
// FloatNear(first field, max_abs_error) matches the second field.
inline internal::FloatingEq2Matcher<float> FloatNear(float max_abs_error) {
  return internal::FloatingEq2Matcher<float>(max_abs_error);
}

// Creates a polymorphic matcher that matches a 2-tuple where
// DoubleNear(first field, max_abs_error) matches the second field.
inline internal::FloatingEq2Matcher<double> DoubleNear(double max_abs_error) {
  return internal::FloatingEq2Matcher<double>(max_abs_error);
}

// Creates a polymorphic matcher that matches a 2-tuple where
// FloatNear(first field, max_abs_error) matches the second field with NaN
// equality.
inline internal::FloatingEq2Matcher<float> NanSensitiveFloatNear(
    float max_abs_error) {
  return internal::FloatingEq2Matcher<float>(max_abs_error, true);
}

// Creates a polymorphic matcher that matches a 2-tuple where
// DoubleNear(first field, max_abs_error) matches the second field with NaN
// equality.
inline internal::FloatingEq2Matcher<double> NanSensitiveDoubleNear(
    double max_abs_error) {
  return internal::FloatingEq2Matcher<double>(max_abs_error, true);
}

// Creates a matcher that matches any value of type T that m doesn't
// match.
template <typename InnerMatcher>
inline internal::NotMatcher<InnerMatcher> Not(InnerMatcher m) {
  return internal::NotMatcher<InnerMatcher>(m);
}

// Returns a matcher that matches anything that satisfies the given
// predicate.  The predicate can be any unary function or functor
// whose return type can be implicitly converted to bool.
template <typename Predicate>
inline PolymorphicMatcher<internal::TrulyMatcher<Predicate>> Truly(
    Predicate pred) {
  return MakePolymorphicMatcher(internal::TrulyMatcher<Predicate>(pred));
}

// Returns a matcher that matches the container size. The container must
// support both size() and size_type which all STL-like containers provide.
// Note that the parameter 'size' can be a value of type size_type as well as
// matcher. For instance:
//   EXPECT_THAT(container, SizeIs(2));     // Checks container has 2 elements.
//   EXPECT_THAT(container, SizeIs(Le(2));  // Checks container has at most 2.
template <typename SizeMatcher>
inline internal::SizeIsMatcher<SizeMatcher> SizeIs(
    const SizeMatcher& size_matcher) {
  return internal::SizeIsMatcher<SizeMatcher>(size_matcher);
}

// Returns a matcher that matches the distance between the container's begin()
// iterator and its end() iterator, i.e. the size of the container. This matcher
// can be used instead of SizeIs with containers such as std::forward_list which
// do not implement size(). The container must provide const_iterator (with
// valid iterator_traits), begin() and end().
template <typename DistanceMatcher>
inline internal::BeginEndDistanceIsMatcher<DistanceMatcher> BeginEndDistanceIs(
    const DistanceMatcher& distance_matcher) {
  return internal::BeginEndDistanceIsMatcher<DistanceMatcher>(distance_matcher);
}

// Returns a matcher that matches an equal container.
// This matcher behaves like Eq(), but in the event of mismatch lists the
// values that are included in one container but not the other. (Duplicate
// values and order differences are not explained.)
template <typename Container>
inline PolymorphicMatcher<
    internal::ContainerEqMatcher<typename std::remove_const<Container>::type>>
ContainerEq(const Container& rhs) {
  return MakePolymorphicMatcher(internal::ContainerEqMatcher<Container>(rhs));
}

// Returns a matcher that matches a container that, when sorted using
// the given comparator, matches container_matcher.
template <typename Comparator, typename ContainerMatcher>
inline internal::WhenSortedByMatcher<Comparator, ContainerMatcher> WhenSortedBy(
    const Comparator& comparator, const ContainerMatcher& container_matcher) {
  return internal::WhenSortedByMatcher<Comparator, ContainerMatcher>(
      comparator, container_matcher);
}

// Returns a matcher that matches a container that, when sorted using
// the < operator, matches container_matcher.
template <typename ContainerMatcher>
inline internal::WhenSortedByMatcher<internal::LessComparator, ContainerMatcher>
WhenSorted(const ContainerMatcher& container_matcher) {
  return internal::WhenSortedByMatcher<internal::LessComparator,
                                       ContainerMatcher>(
      internal::LessComparator(), container_matcher);
}

// Matches an STL-style container or a native array that contains the
// same number of elements as in rhs, where its i-th element and rhs's
// i-th element (as a pair) satisfy the given pair matcher, for all i.
// TupleMatcher must be able to be safely cast to Matcher<std::tuple<const
// T1&, const T2&> >, where T1 and T2 are the types of elements in the
// LHS container and the RHS container respectively.
template <typename TupleMatcher, typename Container>
inline internal::PointwiseMatcher<TupleMatcher,
                                  typename std::remove_const<Container>::type>
Pointwise(const TupleMatcher& tuple_matcher, const Container& rhs) {
  return internal::PointwiseMatcher<TupleMatcher, Container>(tuple_matcher,
                                                             rhs);
}

// Supports the Pointwise(m, {a, b, c}) syntax.
template <typename TupleMatcher, typename T>
inline internal::PointwiseMatcher<TupleMatcher, std::vector<T>> Pointwise(
    const TupleMatcher& tuple_matcher, std::initializer_list<T> rhs) {
  return Pointwise(tuple_matcher, std::vector<T>(rhs));
}

// UnorderedPointwise(pair_matcher, rhs) matches an STL-style
// container or a native array that contains the same number of
// elements as in rhs, where in some permutation of the container, its
// i-th element and rhs's i-th element (as a pair) satisfy the given
// pair matcher, for all i.  Tuple2Matcher must be able to be safely
// cast to Matcher<std::tuple<const T1&, const T2&> >, where T1 and T2 are
// the types of elements in the LHS container and the RHS container
// respectively.
//
// This is like Pointwise(pair_matcher, rhs), except that the element
// order doesn't matter.
template <typename Tuple2Matcher, typename RhsContainer>
inline internal::UnorderedElementsAreArrayMatcher<
    typename internal::BoundSecondMatcher<
        Tuple2Matcher,
        typename internal::StlContainerView<
            typename std::remove_const<RhsContainer>::type>::type::value_type>>
UnorderedPointwise(const Tuple2Matcher& tuple2_matcher,
                   const RhsContainer& rhs_container) {
  // RhsView allows the same code to handle RhsContainer being a
  // STL-style container and it being a native C-style array.
  typedef typename internal::StlContainerView<RhsContainer> RhsView;
  typedef typename RhsView::type RhsStlContainer;
  typedef typename RhsStlContainer::value_type Second;
  const RhsStlContainer& rhs_stl_container =
      RhsView::ConstReference(rhs_container);

  // Create a matcher for each element in rhs_container.
  ::std::vector<internal::BoundSecondMatcher<Tuple2Matcher, Second>> matchers;
  for (auto it = rhs_stl_container.begin(); it != rhs_stl_container.end();
       ++it) {
    matchers.push_back(internal::MatcherBindSecond(tuple2_matcher, *it));
  }

  // Delegate the work to UnorderedElementsAreArray().
  return UnorderedElementsAreArray(matchers);
}

// Supports the UnorderedPointwise(m, {a, b, c}) syntax.
template <typename Tuple2Matcher, typename T>
inline internal::UnorderedElementsAreArrayMatcher<
    typename internal::BoundSecondMatcher<Tuple2Matcher, T>>
UnorderedPointwise(const Tuple2Matcher& tuple2_matcher,
                   std::initializer_list<T> rhs) {
  return UnorderedPointwise(tuple2_matcher, std::vector<T>(rhs));
}

// Matches an STL-style container or a native array that contains at
// least one element matching the given value or matcher.
//
// Examples:
//   ::std::set<int> page_ids;
//   page_ids.insert(3);
//   page_ids.insert(1);
//   EXPECT_THAT(page_ids, Contains(1));
//   EXPECT_THAT(page_ids, Contains(Gt(2)));
//   EXPECT_THAT(page_ids, Not(Contains(4)));  // See below for Times(0)
//
//   ::std::map<int, size_t> page_lengths;
//   page_lengths[1] = 100;
//   EXPECT_THAT(page_lengths,
//               Contains(::std::pair<const int, size_t>(1, 100)));
//
//   const char* user_ids[] = { "joe", "mike", "tom" };
//   EXPECT_THAT(user_ids, Contains(Eq(::std::string("tom"))));
//
// The matcher supports a modifier `Times` that allows to check for arbitrary
// occurrences including testing for absence with Times(0).
//
// Examples:
//   ::std::vector<int> ids;
//   ids.insert(1);
//   ids.insert(1);
//   ids.insert(3);
//   EXPECT_THAT(ids, Contains(1).Times(2));      // 1 occurs 2 times
//   EXPECT_THAT(ids, Contains(2).Times(0));      // 2 is not present
//   EXPECT_THAT(ids, Contains(3).Times(Ge(1)));  // 3 occurs at least once

template <typename M>
inline internal::ContainsMatcher<M> Contains(M matcher) {
  return internal::ContainsMatcher<M>(matcher);
}

// IsSupersetOf(iterator_first, iterator_last)
// IsSupersetOf(pointer, count)
// IsSupersetOf(array)
// IsSupersetOf(container)
// IsSupersetOf({e1, e2, ..., en})
//
// IsSupersetOf() verifies that a surjective partial mapping onto a collection
// of matchers exists. In other words, a container matches
// IsSupersetOf({e1, ..., en}) if and only if there is a permutation
// {y1, ..., yn} of some of the container's elements where y1 matches e1,
// ..., and yn matches en. Obviously, the size of the container must be >= n
// in order to have a match. Examples:
//
// - {1, 2, 3} matches IsSupersetOf({Ge(3), Ne(0)}), as 3 matches Ge(3) and
//   1 matches Ne(0).
// - {1, 2} doesn't match IsSupersetOf({Eq(1), Lt(2)}), even though 1 matches
//   both Eq(1) and Lt(2). The reason is that different matchers must be used
//   for elements in different slots of the container.
// - {1, 1, 2} matches IsSupersetOf({Eq(1), Lt(2)}), as (the first) 1 matches
//   Eq(1) and (the second) 1 matches Lt(2).
// - {1, 2, 3} matches IsSupersetOf(Gt(1), Gt(1)), as 2 matches (the first)
//   Gt(1) and 3 matches (the second) Gt(1).
//
// The matchers can be specified as an array, a pointer and count, a container,
// an initializer list, or an STL iterator range. In each of these cases, the
// underlying matchers can be either values or matchers.

template <typename Iter>
inline internal::UnorderedElementsAreArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
IsSupersetOf(Iter first, Iter last) {
  typedef typename ::std::iterator_traits<Iter>::value_type T;
  return internal::UnorderedElementsAreArrayMatcher<T>(
      internal::UnorderedMatcherRequire::Superset, first, last);
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSupersetOf(
    const T* pointer, size_t count) {
  return IsSupersetOf(pointer, pointer + count);
}

template <typename T, size_t N>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSupersetOf(
    const T (&array)[N]) {
  return IsSupersetOf(array, N);
}

template <typename Container>
inline internal::UnorderedElementsAreArrayMatcher<
    typename Container::value_type>
IsSupersetOf(const Container& container) {
  return IsSupersetOf(container.begin(), container.end());
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSupersetOf(
    ::std::initializer_list<T> xs) {
  return IsSupersetOf(xs.begin(), xs.end());
}

// IsSubsetOf(iterator_first, iterator_last)
// IsSubsetOf(pointer, count)
// IsSubsetOf(array)
// IsSubsetOf(container)
// IsSubsetOf({e1, e2, ..., en})
//
// IsSubsetOf() verifies that an injective mapping onto a collection of matchers
// exists.  In other words, a container matches IsSubsetOf({e1, ..., en}) if and
// only if there is a subset of matchers {m1, ..., mk} which would match the
// container using UnorderedElementsAre.  Obviously, the size of the container
// must be <= n in order to have a match. Examples:
//
// - {1} matches IsSubsetOf({Gt(0), Lt(0)}), as 1 matches Gt(0).
// - {1, -1} matches IsSubsetOf({Lt(0), Gt(0)}), as 1 matches Gt(0) and -1
//   matches Lt(0).
// - {1, 2} doesn't matches IsSubsetOf({Gt(0), Lt(0)}), even though 1 and 2 both
//   match Gt(0). The reason is that different matchers must be used for
//   elements in different slots of the container.
//
// The matchers can be specified as an array, a pointer and count, a container,
// an initializer list, or an STL iterator range. In each of these cases, the
// underlying matchers can be either values or matchers.

template <typename Iter>
inline internal::UnorderedElementsAreArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
IsSubsetOf(Iter first, Iter last) {
  typedef typename ::std::iterator_traits<Iter>::value_type T;
  return internal::UnorderedElementsAreArrayMatcher<T>(
      internal::UnorderedMatcherRequire::Subset, first, last);
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSubsetOf(
    const T* pointer, size_t count) {
  return IsSubsetOf(pointer, pointer + count);
}

template <typename T, size_t N>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSubsetOf(
    const T (&array)[N]) {
  return IsSubsetOf(array, N);
}

template <typename Container>
inline internal::UnorderedElementsAreArrayMatcher<
    typename Container::value_type>
IsSubsetOf(const Container& container) {
  return IsSubsetOf(container.begin(), container.end());
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSubsetOf(
    ::std::initializer_list<T> xs) {
  return IsSubsetOf(xs.begin(), xs.end());
}

// Matches an STL-style container or a native array that contains only
// elements matching the given value or matcher.
//
// Each(m) is semantically equivalent to `Not(Contains(Not(m)))`. Only
// the messages are different.
//
// Examples:
//   ::std::set<int> page_ids;
//   // Each(m) matches an empty container, regardless of what m is.
//   EXPECT_THAT(page_ids, Each(Eq(1)));
//   EXPECT_THAT(page_ids, Each(Eq(77)));
//
//   page_ids.insert(3);
//   EXPECT_THAT(page_ids, Each(Gt(0)));
//   EXPECT_THAT(page_ids, Not(Each(Gt(4))));
//   page_ids.insert(1);
//   EXPECT_THAT(page_ids, Not(Each(Lt(2))));
//
//   ::std::map<int, size_t> page_lengths;
//   page_lengths[1] = 100;
//   page_lengths[2] = 200;
//   page_lengths[3] = 300;
//   EXPECT_THAT(page_lengths, Not(Each(Pair(1, 100))));
//   EXPECT_THAT(page_lengths, Each(Key(Le(3))));
//
//   const char* user_ids[] = { "joe", "mike", "tom" };
//   EXPECT_THAT(user_ids, Not(Each(Eq(::std::string("tom")))));
template <typename M>
inline internal::EachMatcher<M> Each(M matcher) {
  return internal::EachMatcher<M>(matcher);
}

// Key(inner_matcher) matches an std::pair whose 'first' field matches
// inner_matcher.  For example, Contains(Key(Ge(5))) can be used to match an
// std::map that contains at least one element whose key is >= 5.
template <typename M>
inline internal::KeyMatcher<M> Key(M inner_matcher) {
  return internal::KeyMatcher<M>(inner_matcher);
}

// Pair(first_matcher, second_matcher) matches a std::pair whose 'first' field
// matches first_matcher and whose 'second' field matches second_matcher.  For
// example, EXPECT_THAT(map_type, ElementsAre(Pair(Ge(5), "foo"))) can be used
// to match a std::map<int, string> that contains exactly one element whose key
// is >= 5 and whose value equals "foo".
template <typename FirstMatcher, typename SecondMatcher>
inline internal::PairMatcher<FirstMatcher, SecondMatcher> Pair(
    FirstMatcher first_matcher, SecondMatcher second_matcher) {
  return internal::PairMatcher<FirstMatcher, SecondMatcher>(first_matcher,
                                                            second_matcher);
}

namespace no_adl {
// Conditional() creates a matcher that conditionally uses either the first or
// second matcher provided. For example, we could create an `equal if, and only
// if' matcher using the Conditional wrapper as follows:
//
//   EXPECT_THAT(result, Conditional(condition, Eq(expected), Ne(expected)));
template <typename MatcherTrue, typename MatcherFalse>
internal::ConditionalMatcher<MatcherTrue, MatcherFalse> Conditional(
    bool condition, MatcherTrue matcher_true, MatcherFalse matcher_false) {
  return internal::ConditionalMatcher<MatcherTrue, MatcherFalse>(
      condition, std::move(matcher_true), std::move(matcher_false));
}

// FieldsAre(matchers...) matches piecewise the fields of compatible structs.
// These include those that support `get<I>(obj)`, and when structured bindings
// are enabled any class that supports them.
// In particular, `std::tuple`, `std::pair`, `std::array` and aggregate types.
template <typename... M>
internal::FieldsAreMatcher<typename std::decay<M>::type...> FieldsAre(
    M&&... matchers) {
  return internal::FieldsAreMatcher<typename std::decay<M>::type...>(
      std::forward<M>(matchers)...);
}

// Creates a matcher that matches a pointer (raw or smart) that matches
// inner_matcher.
template <typename InnerMatcher>
inline internal::PointerMatcher<InnerMatcher> Pointer(
    const InnerMatcher& inner_matcher) {
  return internal::PointerMatcher<InnerMatcher>(inner_matcher);
}

// Creates a matcher that matches an object that has an address that matches
// inner_matcher.
template <typename InnerMatcher>
inline internal::AddressMatcher<InnerMatcher> Address(
    const InnerMatcher& inner_matcher) {
  return internal::AddressMatcher<InnerMatcher>(inner_matcher);
}

// Matches a base64 escaped string, when the unescaped string matches the
// internal matcher.
template <typename MatcherType>
internal::WhenBase64UnescapedMatcher WhenBase64Unescaped(
    const MatcherType& internal_matcher) {
  return internal::WhenBase64UnescapedMatcher(internal_matcher);
}
}  // namespace no_adl

// Returns a predicate that is satisfied by anything that matches the
// given matcher.
template <typename M>
inline internal::MatcherAsPredicate<M> Matches(M matcher) {
  return internal::MatcherAsPredicate<M>(matcher);
}

// Returns true if and only if the value matches the matcher.
template <typename T, typename M>
inline bool Value(const T& value, M matcher) {
  return testing::Matches(matcher)(value);
}

// Matches the value against the given matcher and explains the match
// result to listener.
template <typename T, typename M>
inline bool ExplainMatchResult(M matcher, const T& value,
                               MatchResultListener* listener) {
  return SafeMatcherCast<const T&>(matcher).MatchAndExplain(value, listener);
}

// Returns a string representation of the given matcher.  Useful for description
// strings of matchers defined using MATCHER_P* macros that accept matchers as
// their arguments.  For example:
//
// MATCHER_P(XAndYThat, matcher,
//           "X that " + DescribeMatcher<int>(matcher, negation) +
//               (negation ? " or" : " and") + " Y that " +
//               DescribeMatcher<double>(matcher, negation)) {
//   return ExplainMatchResult(matcher, arg.x(), result_listener) &&
//          ExplainMatchResult(matcher, arg.y(), result_listener);
// }
template <typename T, typename M>
std::string DescribeMatcher(const M& matcher, bool negation = false) {
  ::std::stringstream ss;
  Matcher<T> monomorphic_matcher = SafeMatcherCast<T>(matcher);
  if (negation) {
    monomorphic_matcher.DescribeNegationTo(&ss);
  } else {
    monomorphic_matcher.DescribeTo(&ss);
  }
  return ss.str();
}

template <typename... Args>
internal::ElementsAreMatcher<
    std::tuple<typename std::decay<const Args&>::type...>>
ElementsAre(const Args&... matchers) {
  return internal::ElementsAreMatcher<
      std::tuple<typename std::decay<const Args&>::type...>>(
      std::make_tuple(matchers...));
}

template <typename... Args>
internal::UnorderedElementsAreMatcher<
    std::tuple<typename std::decay<const Args&>::type...>>
UnorderedElementsAre(const Args&... matchers) {
  return internal::UnorderedElementsAreMatcher<
      std::tuple<typename std::decay<const Args&>::type...>>(
      std::make_tuple(matchers...));
}

// Define variadic matcher versions.
template <typename... Args>
internal::AllOfMatcher<typename std::decay<const Args&>::type...> AllOf(
    const Args&... matchers) {
  return internal::AllOfMatcher<typename std::decay<const Args&>::type...>(
      matchers...);
}

template <typename... Args>
internal::AnyOfMatcher<typename std::decay<const Args&>::type...> AnyOf(
    const Args&... matchers) {
  return internal::AnyOfMatcher<typename std::decay<const Args&>::type...>(
      matchers...);
}

// AnyOfArray(array)
// AnyOfArray(pointer, count)
// AnyOfArray(container)
// AnyOfArray({ e1, e2, ..., en })
// AnyOfArray(iterator_first, iterator_last)
//
// AnyOfArray() verifies whether a given value matches any member of a
// collection of matchers.
//
// AllOfArray(array)
// AllOfArray(pointer, count)
// AllOfArray(container)
// AllOfArray({ e1, e2, ..., en })
// AllOfArray(iterator_first, iterator_last)
//
// AllOfArray() verifies whether a given value matches all members of a
// collection of matchers.
//
// The matchers can be specified as an array, a pointer and count, a container,
// an initializer list, or an STL iterator range. In each of these cases, the
// underlying matchers can be either values or matchers.

template <typename Iter>
inline internal::AnyOfArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
AnyOfArray(Iter first, Iter last) {
  return internal::AnyOfArrayMatcher<
      typename ::std::iterator_traits<Iter>::value_type>(first, last);
}

template <typename Iter>
inline internal::AllOfArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
AllOfArray(Iter first, Iter last) {
  return internal::AllOfArrayMatcher<
      typename ::std::iterator_traits<Iter>::value_type>(first, last);
}

template <typename T>
inline internal::AnyOfArrayMatcher<T> AnyOfArray(const T* ptr, size_t count) {
  return AnyOfArray(ptr, ptr + count);
}

template <typename T>
inline internal::AllOfArrayMatcher<T> AllOfArray(const T* ptr, size_t count) {
  return AllOfArray(ptr, ptr + count);
}

template <typename T, size_t N>
inline internal::AnyOfArrayMatcher<T> AnyOfArray(const T (&array)[N]) {
  return AnyOfArray(array, N);
}

template <typename T, size_t N>
inline internal::AllOfArrayMatcher<T> AllOfArray(const T (&array)[N]) {
  return AllOfArray(array, N);
}

template <typename Container>
inline internal::AnyOfArrayMatcher<typename Container::value_type> AnyOfArray(
    const Container& container) {
  return AnyOfArray(container.begin(), container.end());
}

template <typename Container>
inline internal::AllOfArrayMatcher<typename Container::value_type> AllOfArray(
    const Container& container) {
  return AllOfArray(container.begin(), container.end());
}

template <typename T>
inline internal::AnyOfArrayMatcher<T> AnyOfArray(
    ::std::initializer_list<T> xs) {
  return AnyOfArray(xs.begin(), xs.end());
}

template <typename T>
inline internal::AllOfArrayMatcher<T> AllOfArray(
    ::std::initializer_list<T> xs) {
  return AllOfArray(xs.begin(), xs.end());
}

// Args<N1, N2, ..., Nk>(a_matcher) matches a tuple if the selected
// fields of it matches a_matcher.  C++ doesn't support default
// arguments for function templates, so we have to overload it.
template <size_t... k, typename InnerMatcher>
internal::ArgsMatcher<typename std::decay<InnerMatcher>::type, k...> Args(
    InnerMatcher&& matcher) {
  return internal::ArgsMatcher<typename std::decay<InnerMatcher>::type, k...>(
      std::forward<InnerMatcher>(matcher));
}

// AllArgs(m) is a synonym of m.  This is useful in
//
//   EXPECT_CALL(foo, Bar(_, _)).With(AllArgs(Eq()));
//
// which is easier to read than
//
//   EXPECT_CALL(foo, Bar(_, _)).With(Eq());
template <typename InnerMatcher>
inline InnerMatcher AllArgs(const InnerMatcher& matcher) {
  return matcher;
}

// Returns a matcher that matches the value of an optional<> type variable.
// The matcher implementation only uses '!arg' and requires that the optional<>
// type has a 'value_type' member type and that '*arg' is of type 'value_type'
// and is printable using 'PrintToString'. It is compatible with
// std::optional/std::experimental::optional.
// Note that to compare an optional type variable against nullopt you should
// use Eq(nullopt) and not Eq(Optional(nullopt)). The latter implies that the
// optional value contains an optional itself.
template <typename ValueMatcher>
inline internal::OptionalMatcher<ValueMatcher> Optional(
    const ValueMatcher& value_matcher) {
  return internal::OptionalMatcher<ValueMatcher>(value_matcher);
}

// Returns a matcher that matches the value of a absl::any type variable.
template <typename T>
PolymorphicMatcher<internal::any_cast_matcher::AnyCastMatcher<T>> AnyWith(
    const Matcher<const T&>& matcher) {
  return MakePolymorphicMatcher(
      internal::any_cast_matcher::AnyCastMatcher<T>(matcher));
}

// Returns a matcher that matches the value of a variant<> type variable.
// The matcher implementation uses ADL to find the holds_alternative and get
// functions.
// It is compatible with std::variant.
template <typename T>
PolymorphicMatcher<internal::variant_matcher::VariantMatcher<T>> VariantWith(
    const Matcher<const T&>& matcher) {
  return MakePolymorphicMatcher(
      internal::variant_matcher::VariantMatcher<T>(matcher));
}

#if GTEST_HAS_EXCEPTIONS

// Anything inside the `internal` namespace is internal to the implementation
// and must not be used in user code!
namespace internal {

class WithWhatMatcherImpl {
 public:
  WithWhatMatcherImpl(Matcher<std::string> matcher)
      : matcher_(std::move(matcher)) {}

  void DescribeTo(std::ostream* os) const {
    *os << "contains .what() that ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "contains .what() that does not ";
    matcher_.DescribeTo(os);
  }

  template <typename Err>
  bool MatchAndExplain(const Err& err, MatchResultListener* listener) const {
    *listener << "which contains .what() (of value = " << err.what()
              << ") that ";
    return matcher_.MatchAndExplain(err.what(), listener);
  }

 private:
  const Matcher<std::string> matcher_;
};

inline PolymorphicMatcher<WithWhatMatcherImpl> WithWhat(
    Matcher<std::string> m) {
  return MakePolymorphicMatcher(WithWhatMatcherImpl(std::move(m)));
}

template <typename Err>
class ExceptionMatcherImpl {
  class NeverThrown {
   public:
    const char* what() const noexcept {
      return "this exception should never be thrown";
    }
  };

  // If the matchee raises an exception of a wrong type, we'd like to
  // catch it and print its message and type. To do that, we add an additional
  // catch clause:
  //
  //     try { ... }
  //     catch (const Err&) { /* an expected exception */ }
  //     catch (const std::exception&) { /* exception of a wrong type */ }
  //
  // However, if the `Err` itself is `std::exception`, we'd end up with two
  // identical `catch` clauses:
  //
  //     try { ... }
  //     catch (const std::exception&) { /* an expected exception */ }
  //     catch (const std::exception&) { /* exception of a wrong type */ }
  //
  // This can cause a warning or an error in some compilers. To resolve
  // the issue, we use a fake error type whenever `Err` is `std::exception`:
  //
  //     try { ... }
  //     catch (const std::exception&) { /* an expected exception */ }
  //     catch (const NeverThrown&) { /* exception of a wrong type */ }
  using DefaultExceptionType = typename std::conditional<
      std::is_same<typename std::remove_cv<
                       typename std::remove_reference<Err>::type>::type,
                   std::exception>::value,
      const NeverThrown&, const std::exception&>::type;

 public:
  ExceptionMatcherImpl(Matcher<const Err&> matcher)
      : matcher_(std::move(matcher)) {}

  void DescribeTo(std::ostream* os) const {
    *os << "throws an exception which is a " << GetTypeName<Err>();
    *os << " which ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "throws an exception which is not a " << GetTypeName<Err>();
    *os << " which ";
    matcher_.DescribeNegationTo(os);
  }

  template <typename T>
  bool MatchAndExplain(T&& x, MatchResultListener* listener) const {
    try {
      (void)(std::forward<T>(x)());
    } catch (const Err& err) {
      *listener << "throws an exception which is a " << GetTypeName<Err>();
      *listener << " ";
      return matcher_.MatchAndExplain(err, listener);
    } catch (DefaultExceptionType err) {
#if GTEST_HAS_RTTI
      *listener << "throws an exception of type " << GetTypeName(typeid(err));
      *listener << " ";
#else
      *listener << "throws an std::exception-derived type ";
#endif
      *listener << "with description \"" << err.what() << "\"";
      return false;
    } catch (...) {
      *listener << "throws an exception of an unknown type";
      return false;
    }

    *listener << "does not throw any exception";
    return false;
  }

 private:
  const Matcher<const Err&> matcher_;
};

}  // namespace internal

// Throws()
// Throws(exceptionMatcher)
// ThrowsMessage(messageMatcher)
//
// This matcher accepts a callable and verifies that when invoked, it throws
// an exception with the given type and properties.
//
// Examples:
//
//   EXPECT_THAT(
//       []() { throw std::runtime_error("message"); },
//       Throws<std::runtime_error>());
//
//   EXPECT_THAT(
//       []() { throw std::runtime_error("message"); },
//       ThrowsMessage<std::runtime_error>(HasSubstr("message")));
//
//   EXPECT_THAT(
//       []() { throw std::runtime_error("message"); },
//       Throws<std::runtime_error>(
//           Property(&std::runtime_error::what, HasSubstr("message"))));

template <typename Err>
PolymorphicMatcher<internal::ExceptionMatcherImpl<Err>> Throws() {
  return MakePolymorphicMatcher(
      internal::ExceptionMatcherImpl<Err>(A<const Err&>()));
}

template <typename Err, typename ExceptionMatcher>
PolymorphicMatcher<internal::ExceptionMatcherImpl<Err>> Throws(
    const ExceptionMatcher& exception_matcher) {
  // Using matcher cast allows users to pass a matcher of a more broad type.
  // For example user may want to pass Matcher<std::exception>
  // to Throws<std::runtime_error>, or Matcher<int64> to Throws<int32>.
  return MakePolymorphicMatcher(internal::ExceptionMatcherImpl<Err>(
      SafeMatcherCast<const Err&>(exception_matcher)));
}

template <typename Err, typename MessageMatcher>
PolymorphicMatcher<internal::ExceptionMatcherImpl<Err>> ThrowsMessage(
    MessageMatcher&& message_matcher) {
  static_assert(std::is_base_of<std::exception, Err>::value,
                "expected an std::exception-derived type");
  return Throws<Err>(internal::WithWhat(
      MatcherCast<std::string>(std::forward<MessageMatcher>(message_matcher))));
}

#endif  // GTEST_HAS_EXCEPTIONS

// These macros allow using matchers to check values in Google Test
// tests.  ASSERT_THAT(value, matcher) and EXPECT_THAT(value, matcher)
// succeed if and only if the value matches the matcher.  If the assertion
// fails, the value and the description of the matcher will be printed.
#define ASSERT_THAT(value, matcher) \
  ASSERT_PRED_FORMAT1(              \
      ::testing::internal::MakePredicateFormatterFromMatcher(matcher), value)
#define EXPECT_THAT(value, matcher) \
  EXPECT_PRED_FORMAT1(              \
      ::testing::internal::MakePredicateFormatterFromMatcher(matcher), value)

// MATCHER* macros itself are listed below.
#define MATCHER(name, description)                                            \
  class name##Matcher                                                         \
      : public ::testing::internal::MatcherBaseImpl<name##Matcher> {          \
   public:                                                                    \
    template <typename arg_type>                                              \
    class gmock_Impl : public ::testing::MatcherInterface<const arg_type&> {  \
     public:                                                                  \
      gmock_Impl() {}                                                         \
      bool MatchAndExplain(                                                   \
          const arg_type& arg,                                                \
          ::testing::MatchResultListener* result_listener) const override;    \
      void DescribeTo(::std::ostream* gmock_os) const override {              \
        *gmock_os << FormatDescription(false);                                \
      }                                                                       \
      void DescribeNegationTo(::std::ostream* gmock_os) const override {      \
        *gmock_os << FormatDescription(true);                                 \
      }                                                                       \
                                                                              \
     private:                                                                 \
      ::std::string FormatDescription(bool negation) const {                  \
        /* NOLINTNEXTLINE readability-redundant-string-init */                \
        ::std::string gmock_description = (description);                      \
        if (!gmock_description.empty()) {                                     \
          return gmock_description;                                           \
        }                                                                     \
        return ::testing::internal::FormatMatcherDescription(negation, #name, \
                                                             {}, {});         \
      }                                                                       \
    };                                                                        \
  };                                                                          \
  inline name##Matcher GMOCK_INTERNAL_WARNING_PUSH()                          \
      GMOCK_INTERNAL_WARNING_CLANG(ignored, "-Wunused-function")              \
          GMOCK_INTERNAL_WARNING_CLANG(ignored, "-Wunused-member-function")   \
              name GMOCK_INTERNAL_WARNING_POP()() {                           \
    return {};                                                                \
  }                                                                           \
  template <typename arg_type>                                                \
  bool name##Matcher::gmock_Impl<arg_type>::MatchAndExplain(                  \
      const arg_type& arg,                                                    \
      GTEST_INTERNAL_ATTRIBUTE_MAYBE_UNUSED ::testing::MatchResultListener*   \
          result_listener) const

#define MATCHER_P(name, p0, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP, description, (#p0), (p0))
#define MATCHER_P2(name, p0, p1, description)                            \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP2, description, (#p0, #p1), \
                         (p0, p1))
#define MATCHER_P3(name, p0, p1, p2, description)                             \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP3, description, (#p0, #p1, #p2), \
                         (p0, p1, p2))
#define MATCHER_P4(name, p0, p1, p2, p3, description)        \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP4, description, \
                         (#p0, #p1, #p2, #p3), (p0, p1, p2, p3))
#define MATCHER_P5(name, p0, p1, p2, p3, p4, description)    \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP5, description, \
                         (#p0, #p1, #p2, #p3, #p4), (p0, p1, p2, p3, p4))
#define MATCHER_P6(name, p0, p1, p2, p3, p4, p5, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP6, description,  \
                         (#p0, #p1, #p2, #p3, #p4, #p5),      \
                         (p0, p1, p2, p3, p4, p5))
#define MATCHER_P7(name, p0, p1, p2, p3, p4, p5, p6, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP7, description,      \
                         (#p0, #p1, #p2, #p3, #p4, #p5, #p6),     \
                         (p0, p1, p2, p3, p4, p5, p6))
#define MATCHER_P8(name, p0, p1, p2, p3, p4, p5, p6, p7, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP8, description,          \
                         (#p0, #p1, #p2, #p3, #p4, #p5, #p6, #p7),    \
                         (p0, p1, p2, p3, p4, p5, p6, p7))
#define MATCHER_P9(name, p0, p1, p2, p3, p4, p5, p6, p7, p8, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP9, description,              \
                         (#p0, #p1, #p2, #p3, #p4, #p5, #p6, #p7, #p8),   \
                         (p0, p1, p2, p3, p4, p5, p6, p7, p8))
#define MATCHER_P10(name, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP10, description,                  \
                         (#p0, #p1, #p2, #p3, #p4, #p5, #p6, #p7, #p8, #p9),   \
                         (p0, p1, p2, p3, p4, p5, p6, p7, p8, p9))

#define GMOCK_INTERNAL_MATCHER(name, full_name, description, arg_names, args)  \
  template <GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAMS(args)>                      \
  class full_name : public ::testing::internal::MatcherBaseImpl<               \
                        full_name<GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args)>> { \
   public:                                                                     \
    using full_name::MatcherBaseImpl::MatcherBaseImpl;                         \
    template <typename arg_type>                                               \
    class gmock_Impl : public ::testing::MatcherInterface<const arg_type&> {   \
     public:                                                                   \
      explicit gmock_Impl(GMOCK_INTERNAL_MATCHER_FUNCTION_ARGS(args))          \
          : GMOCK_INTERNAL_MATCHER_FORWARD_ARGS(args) {}                       \
      bool MatchAndExplain(                                                    \
          const arg_type& arg,                                                 \
          ::testing::MatchResultListener* result_listener) const override;     \
      void DescribeTo(::std::ostream* gmock_os) const override {               \
        *gmock_os << FormatDescription(false);                                 \
      }                                                                        \
      void DescribeNegationTo(::std::ostream* gmock_os) const override {       \
        *gmock_os << FormatDescription(true);                                  \
      }                                                                        \
      GMOCK_INTERNAL_MATCHER_MEMBERS(args)                                     \
                                                                               \
     private:                                                                  \
      ::std::string FormatDescription(bool negation) const {                   \
        ::std::string gmock_description;                                       \
        gmock_description = (description);                                     \
        if (!gmock_description.empty()) {                                      \
          return gmock_description;                                            \
        }                                                                      \
        return ::testing::internal::FormatMatcherDescription(                  \
            negation, #name, {GMOCK_PP_REMOVE_PARENS(arg_names)},              \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(      \
                ::std::tuple<GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args)>(        \
                    GMOCK_INTERNAL_MATCHER_MEMBERS_USAGE(args))));             \
      }                                                                        \
    };                                                                         \
  };                                                                           \
  template <GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAMS(args)>                      \
  inline full_name<GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args)> name(             \
      GMOCK_INTERNAL_MATCHER_FUNCTION_ARGS(args)) {                            \
    return full_name<GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args)>(                \
        GMOCK_INTERNAL_MATCHER_ARGS_USAGE(args));                              \
  }                                                                            \
  template <GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAMS(args)>                      \
  template <typename arg_type>                                                 \
  bool full_name<GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args)>::                   \
      gmock_Impl<arg_type>::MatchAndExplain(                                   \
          const arg_type& arg,                                                 \
          GTEST_INTERNAL_ATTRIBUTE_MAYBE_UNUSED ::testing::                    \
              MatchResultListener* result_listener) const

#define GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAMS(args) \
  GMOCK_PP_TAIL(                                     \
      GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAM, , args))
#define GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAM(i_unused, data_unused, arg) \
  , typename arg##_type

#define GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_TYPE_PARAM, , args))
#define GMOCK_INTERNAL_MATCHER_TYPE_PARAM(i_unused, data_unused, arg) \
  , arg##_type

#define GMOCK_INTERNAL_MATCHER_FUNCTION_ARGS(args) \
  GMOCK_PP_TAIL(dummy_first GMOCK_PP_FOR_EACH(     \
      GMOCK_INTERNAL_MATCHER_FUNCTION_ARG, , args))
#define GMOCK_INTERNAL_MATCHER_FUNCTION_ARG(i, data_unused, arg) \
  , arg##_type gmock_p##i

#define GMOCK_INTERNAL_MATCHER_FORWARD_ARGS(args) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_FORWARD_ARG, , args))
#define GMOCK_INTERNAL_MATCHER_FORWARD_ARG(i, data_unused, arg) \
  , arg(::std::forward<arg##_type>(gmock_p##i))

#define GMOCK_INTERNAL_MATCHER_MEMBERS(args) \
  GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_MEMBER, , args)
#define GMOCK_INTERNAL_MATCHER_MEMBER(i_unused, data_unused, arg) \
  const arg##_type arg;

#define GMOCK_INTERNAL_MATCHER_MEMBERS_USAGE(args) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_MEMBER_USAGE, , args))
#define GMOCK_INTERNAL_MATCHER_MEMBER_USAGE(i_unused, data_unused, arg) , arg

#define GMOCK_INTERNAL_MATCHER_ARGS_USAGE(args) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_ARG_USAGE, , args))
#define GMOCK_INTERNAL_MATCHER_ARG_USAGE(i, data_unused, arg_unused) \
  , gmock_p##i

// To prevent ADL on certain functions we put them on a separate namespace.
using namespace no_adl;  // NOLINT

}  // namespace testing

GTEST_DISABLE_MSC_WARNINGS_POP_()  //  4251 5046

// Include any custom callback matchers added by the local installation.
// We must include this header at the end to make sure it can use the
// declarations from this file.
#include "gmock/internal/custom/gmock-matchers.h"

#endif  // GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_
