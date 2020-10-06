// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHILE_HPP_INCLUDED
#define LEXY_DSL_WHILE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
template <typename Branch>
struct _while : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            auto save = reader;
            while (Branch::condition_matcher::match(reader))
            {
                if (!decltype(Branch::then())::matcher::match(reader))
                {
                    reader = LEXY_MOV(save);
                    return false;
                }
            }

            return true;
        }
    };

    template <typename NextParser>
    struct parser
    {
        struct _continuation
        {
            template <typename Handler, typename Reader, typename... Args>
            LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
                typename Handler::result_type
            {
                // After we've parsed then, we try again.
                // Note that, as we're a pattern, we never add additional arguments.
                return parser::parse(handler, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            if constexpr (Branch::has_then)
            {
                if (Branch::condition_matcher::match(reader))
                    // Try another iteration.
                    return Branch::template then_parser<_continuation>::parse(handler, reader,
                                                                              LEXY_FWD(args)...);
                else
                    // Continue with next parser.
                    return NextParser::parse(handler, reader, LEXY_FWD(args)...);
            }
            else
            {
                // Without a then in the branch, we can just repeatedly match the condition and
                // continue. This doesn't require mutual recursion.
                while (Branch::condition_matcher::match(reader))
                {
                }

                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
            }
        }
    };
};

/// Matches the pattern as often as possible.
template <typename Pattern>
LEXY_CONSTEVAL auto while_(Pattern pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _while<decltype(branch(pattern))>{};
}

/// Matches the pattern at least once, then as often as possible.
template <typename Pattern>
LEXY_CONSTEVAL auto while_one(Pattern pattern)
{
    return pattern + while_(pattern);
}

/// Matches then once, then `while_(condition >> then)`.
template <typename Then, typename Condition>
LEXY_CONSTEVAL auto do_while(Then then, Condition condition)
{
    return then + while_(condition >> then);
}
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED

