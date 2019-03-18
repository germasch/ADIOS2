/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#include <iostream>

#include <adios2/helper/TypeList.h>
#include <adios2/helper/adiosType.h>

#include <gtest/gtest.h>

#include "adios2/helper/variant.hpp"
using mpark::variant;

namespace tl = adios2::helper::tl;

TEST(ADIOS2HelperTuple, GetByType)
{
    std::tuple<int, double> tpl = {1, 3.3};
    EXPECT_EQ(adios2::helper::GetByType<int>(tpl), 1);
    EXPECT_EQ(adios2::helper::GetByType<double>(tpl), 3.3);
}

TEST(ADIOS2HelperVariant, Ctor)
{
    using MyVariant = variant<int, double>;
    MyVariant var = 1;
    EXPECT_TRUE(mpark::get<int>(var) == 1);
    var = 3.3;
    EXPECT_EQ(mpark::get<double>(var), 3.3);
}

struct Visitor
{
  template<class T>
  std::string operator()(const T& val)
  {
      return std::to_string(val);
  }
};

struct Visitor2Arg
{
    template <class T, class U>
    std::string operator()(const T &val, const U *ptr)
    {
        return std::to_string(val) + " " + std::to_string(*ptr);
    }
};

struct VisitorExtraArg
{
    template <class T>
    std::string operator()(const T &val) //, const std::string& sfx)
    {
        return std::to_string(val); // + sfx;
  }
};

TEST(ADIOS2HelperVariant, Visit)
{
    using MyVariant = variant<int, double>;
    MyVariant var = 3.3;
    EXPECT_EQ(visit(Visitor{}, var), "3.300000");
    var = 2;
    EXPECT_EQ((visit(Visitor{}, var)), "2");
}

TEST(ADIOS2HelperVariant, Visit2Arg)
{
    using MyVariant = variant<int, double>;
    using ConstPointerT = variant<const int *, const double *>;
    MyVariant var = 3.3;
    double tmp_double = 4.4;
    ConstPointerT ptr = &tmp_double;
    EXPECT_EQ(visit(Visitor2Arg{}, var, ptr), "3.300000 4.400000");
    var = 2;
    int tmp_int = 5;
    ptr = &tmp_int;
    EXPECT_EQ((visit(Visitor2Arg{}, var, ptr)), "2 5");
}

#define DECLTYPE_AUTO auto
#define DECLTYPE_AUTO_RETURN(...)                                              \
    ->decltype(__VA_ARGS__) { return __VA_ARGS__; }

namespace lib = mpark::lib;

#if 0
#if 0
template <typename Visitor>
struct visitor {
  template <typename... Values>
  inline static constexpr bool does_not_handle() {
    return lib::is_invocable<Visitor, Values...>::value;
  }
};

template <typename Visitor, typename... Values>
struct visit_exhaustiveness_check {
  static_assert(visitor<Visitor>::template does_not_handle<Values...>(),
		"`visit` requires the visitor to be exhaustive.");
  
  inline static constexpr DECLTYPE_AUTO invoke(Visitor &&visitor,
					       Values &&... values)
    DECLTYPE_AUTO_RETURN(lib::invoke(lib::forward<Visitor>(visitor),
				     lib::forward<Values>(values)...))
    };

template <typename Visitor>
struct extra_args_visitor {
  Visitor visitor_;
  
  template <typename... Alts>
  inline constexpr DECLTYPE_AUTO operator()(Alts &&... alts) const
    DECLTYPE_AUTO_RETURN(
			 visit_exhaustiveness_check<
			 Visitor,
			 decltype((lib::forward<Alts>(alts).value))...>::
			 invoke(lib::forward<Visitor>(visitor_),
				lib::forward<Alts>(alts).value...))
    };

template <typename Visitor>
inline static constexpr DECLTYPE_AUTO make_extra_args_visitor(Visitor &&visitor)
  DECLTYPE_AUTO_RETURN(extra_args_visitor<Visitor>{lib::forward<Visitor>(visitor)})
#endif

  template <typename Visitor, typename... Args>
  struct extra_args_visitor {
    Visitor visitor_;
    //static_assert(!std::is_same<Visitor, Visitor>::value, "xxx");
    std::tuple<typename std::decay<Args>::type...> args_;
    
    extra_args_visitor(Visitor&& visitor)//, Args&&... args)
    : visitor_{visitor} {}//, args_{std::make_tuple(std::forward<Args>(args)...)} {}

    template <typename... Alts>
    inline constexpr DECLTYPE_AUTO operator()(Alts &&... alts) const
      DECLTYPE_AUTO_RETURN(lib::invoke(visitor_, lib::forward<Alts>(alts)...));
  };


template <typename Visitor>
inline static constexpr DECLTYPE_AUTO make_extra_args_visitor(Visitor &&visitor)
  DECLTYPE_AUTO_RETURN(extra_args_visitor<Visitor>{lib::forward<Visitor>(visitor)})
  
TEST(ADIOS2HelperVariant, VisitExtraArg)
{
    using MyVariant = variant<int, double>;
    MyVariant var = 3.3;
    //    EXPECT_EQ(visit(VisitorExtraArg{}, var, "x"), "3.300000 4.400000");
    var = 2;
    VisitorExtraArg visitor;
    //auto&& extra_arg_visitor = make_extra_args_visitor(visitor);
    auto&& extra_arg_visitor = make_extra_args_visitor(VisitorExtraArg{});
    visit(make_extra_args_visitor(VisitorExtraArg{}), var);
}
#endif

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    return result;
}
