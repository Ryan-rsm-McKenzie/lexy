// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/minus.hpp>

#include "verify.hpp"
#include <lexy/dsl/any.hpp>
#include <lexy/dsl/until.hpp>

TEST_CASE("dsl::operator-")
{
    SUBCASE("basic")
    {
        static constexpr auto rule = until(LEXY_LIT("!")) - LEXY_LIT("aa!");
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == lexy::_detail::string_view(str).end());
                LEXY_VERIFY_CHECK(e.character() == '!');
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::minus_failure> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);
        auto zero = LEXY_VERIFY("!");
        CHECK(zero == 1);

        auto a = LEXY_VERIFY("a!");
        CHECK(a == 2);
        auto aaa = LEXY_VERIFY("aaa!");
        CHECK(aaa == 4);

        auto aa = LEXY_VERIFY("aa!");
        CHECK(aa == -2);
    }
    SUBCASE("sequence")
    {
        static constexpr auto rule = until(LEXY_LIT("!")) - LEXY_LIT("a!") - LEXY_LIT("aa!");
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == lexy::_detail::string_view(str).end());
                LEXY_VERIFY_CHECK(e.character() == '!');
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::minus_failure> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);
        auto zero = LEXY_VERIFY("!");
        CHECK(zero == 1);

        auto a = LEXY_VERIFY("a!");
        CHECK(a == -2);
        auto aa = LEXY_VERIFY("aa!");
        CHECK(aa == -2);

        auto aaa = LEXY_VERIFY("aaa!");
        CHECK(aaa == 4);
    }
    SUBCASE("any")
    {
        static constexpr auto rule = until(LEXY_LIT("!")) - lexy::dsl::any;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == lexy::_detail::string_view(str).end());
                LEXY_VERIFY_CHECK(e.character() == '!');
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::minus_failure> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);
        auto zero = LEXY_VERIFY("!");
        CHECK(zero == -2);

        auto a = LEXY_VERIFY("a!");
        CHECK(a == -2);
        auto aa = LEXY_VERIFY("aa!");
        CHECK(aa == -2);
        auto aaa = LEXY_VERIFY("aaa!");
        CHECK(aaa == -2);
    }
}

