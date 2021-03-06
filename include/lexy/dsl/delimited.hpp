// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DELIMITED_HPP_INCLUDED
#define LEXY_DSL_DELIMITED_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/value.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
/// The reader ends before the closing delimiter was found.
struct missing_delimiter
{
    static LEXY_CONSTEVAL auto name()
    {
        return "missing delimiter";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Close, typename Char, typename Escape>
struct _del : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink      = context.sink();
            auto del_begin = reader.cur();

            using close  = lexy::rule_parser<Close, _list_finish<NextParser, Args...>>;
            using escape = lexy::rule_parser<Escape, _list_sink>;
            while (true)
            {
                // Try to finish parsing the production.
                if (auto result = close::try_parse(context, reader, LEXY_FWD(args)..., sink);
                    result != lexy::rule_try_parse_result::backtracked)
                {
                    // We had a closing delimiter, return that result.
                    return static_cast<bool>(result);
                }
                // Check for missing closing delimiter.
                else if (reader.eof())
                {
                    auto err = lexy::make_error<Reader, lexy::missing_delimiter>(del_begin,
                                                                                 reader.cur());
                    context.error(err);
                    return false;
                }
                // Try to parse an escape sequence.
                else if (auto result = escape::try_parse(context, reader, sink);
                         result != lexy::rule_try_parse_result::backtracked)
                {
                    // We definitely had one, check whether we need to cancel.
                    if (result == lexy::rule_try_parse_result::canceled)
                        return false;
                }
                // Parse the next character.
                else
                {
                    using engine = typename Char::token_engine;
                    if constexpr (lexy::engine_can_fail<engine, Reader>)
                    {
                        auto content_begin = reader.cur();
                        if (auto ec = engine::match(reader); ec != typename engine::error_code())
                        {
                            Char::token_error(context, reader, ec, content_begin);
                            return false;
                        }
                        auto content_end = reader.cur();

                        context.token(Char::token_kind(), content_begin, content_end);
                        sink(lexy::lexeme<Reader>(content_begin, content_end));
                    }
                    else
                    {
                        auto content_begin = reader.cur();
                        engine::match(reader);
                        auto content_end = reader.cur();

                        context.token(Char::token_kind(), content_begin, content_end);
                        sink(lexy::lexeme<Reader>(content_begin, content_end));
                    }
                }
            }

            return false; // unreachable
        }
    };
};
template <typename Close, typename Char>
struct _del<Close, Char, void>
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink      = context.sink();
            auto del_begin = reader.cur();

            using close = lexy::rule_parser<Close, _list_finish<NextParser, Args...>>;
            while (true)
            {
                // Try to finish parsing the production.
                if (auto result = close::try_parse(context, reader, LEXY_FWD(args)..., sink);
                    result != lexy::rule_try_parse_result::backtracked)
                {
                    // We had a closing delimiter, return that result.
                    return static_cast<bool>(result);
                }
                // Check for missing closing delimiter.
                else if (reader.eof())
                {
                    auto err = lexy::make_error<Reader, lexy::missing_delimiter>(del_begin,
                                                                                 reader.cur());
                    context.error(err);
                    return false;
                }
                // Parse the next character.
                else
                {
                    using engine = typename Char::token_engine;
                    if constexpr (lexy::engine_can_fail<engine, Reader>)
                    {
                        auto content_begin = reader.cur();
                        if (auto ec = engine::match(reader); ec != typename engine::error_code())
                        {
                            Char::token_error(context, reader, ec, content_begin);
                            return false;
                        }
                        auto content_end = reader.cur();

                        context.token(Char::token_kind(), content_begin, content_end);
                        sink(lexy::lexeme<Reader>(content_begin, content_end));
                    }
                    else
                    {
                        auto content_begin = reader.cur();
                        engine::match(reader);
                        auto content_end = reader.cur();

                        context.token(Char::token_kind(), content_begin, content_end);
                        sink(lexy::lexeme<Reader>(content_begin, content_end));
                    }
                }
            }

            return false; // unreachable
        }
    };
};

template <typename Open, typename Close>
struct _delim_dsl
{
    /// Sets the whitespace.
    template <typename Ws>
    LEXY_CONSTEVAL auto operator[](Ws ws) const
    {
        auto open = whitespaced(Open{}, ws);
        return _delim_dsl<decltype(open), Close>{};
    }

    /// Sets the content.
    template <typename Char>
    LEXY_CONSTEVAL auto operator()(Char) const
    {
        static_assert(lexy::is_token<Char>);
        return no_whitespace(open() >> _del<Close, Char, void>{});
    }
    template <typename Char, typename Escape>
    LEXY_CONSTEVAL auto operator()(Char, Escape) const
    {
        static_assert(lexy::is_token<Char>);
        static_assert(lexy::is_branch<Escape>);
        return no_whitespace(open() >> _del<Close, Char, Escape>{});
    }

    /// Matches the open delimiter.
    LEXY_CONSTEVAL auto open() const
    {
        return Open{};
    }
    /// Matches the closing delimiter.
    LEXY_CONSTEVAL auto close() const
    {
        // Close never has any whitespace.
        return Close{};
    }
};

/// Parses everything between the two delimiters and captures it.
template <typename Open, typename Close>
LEXY_CONSTEVAL auto delimited(Open, Close)
{
    static_assert(lexy::is_branch<Open> && lexy::is_branch<Close>);
    return _delim_dsl<Open, Close>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
LEXY_CONSTEVAL auto delimited(Delim)
{
    static_assert(lexy::is_branch<Delim>);
    return _delim_dsl<Delim, Delim>{};
}

constexpr auto quoted        = delimited(LEXY_LIT("\""));
constexpr auto triple_quoted = delimited(LEXY_LIT("\"\"\""));

constexpr auto single_quoted = delimited(LEXY_LIT("'"));

constexpr auto backticked        = delimited(LEXY_LIT("`"));
constexpr auto double_backticked = delimited(LEXY_LIT("``"));
constexpr auto triple_backticked = delimited(LEXY_LIT("```"));
} // namespace lexyd

namespace lexy
{
struct invalid_escape_sequence
{
    static LEXY_CONSTEVAL auto name()
    {
        return "invalid escape sequence";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Escape, typename... Branches>
LEXY_CONSTEVAL auto _escape_rule(Branches... branches)
{
    if constexpr (sizeof...(Branches) == 0)
        return Escape{};
    else
        return Escape{} >> (branches | ... | error<lexy::invalid_escape_sequence>);
}

template <typename Engine>
struct _escape_cap : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto begin = reader.cur();
            if (!lexy::engine_try_match<Engine>(reader))
                return lexy::rule_try_parse_result::backtracked;

            return static_cast<lexy::rule_try_parse_result>(
                NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::lexeme(reader, begin)));
        }
    };
};

template <typename Escape, typename... Branches>
struct _escape : decltype(_escape_rule<Escape>(Branches{}...))
{
    /// Adds a generic escape rule.
    template <typename Branch>
    LEXY_CONSTEVAL auto rule(Branch) const
    {
        static_assert(lexy::is_branch<Branch>);
        return _escape<Escape, Branches..., Branch>{};
    }

    /// Adds an escape rule that captures the token.
    template <typename Token>
    LEXY_CONSTEVAL auto capture(Token) const
    {
        static_assert(lexy::is_token<Token>);
        return rule(_escape_cap<typename Token::token_engine>{});
    }

#if LEXY_HAS_NTTP
    /// Adds an escape rule that replaces the escaped string with the replacement.
    template <lexy::_detail::string_literal Str, typename Value>
    LEXY_CONSTEVAL auto lit(Value value) const
    {
        return rule(lexyd::lit<Str> >> value);
    }
    /// Adds an escape rule that replaces the escaped string with itself.
    template <lexy::_detail::string_literal Str>
    LEXY_CONSTEVAL auto lit() const
    {
        return lit<Str>(value_str<Str>);
    }
#endif

    /// Adds an escape rule that replaces the escaped character with the replacement.
    template <auto C, typename Value>
    LEXY_CONSTEVAL auto lit_c(Value value) const
    {
        return rule(lexyd::lit_c<C> >> value);
    }
    /// Adds an escape rule that replaces the escape character with itself.
    template <auto C>
    LEXY_CONSTEVAL auto lit_c() const
    {
        return lit_c<C>(value_c<C>);
    }
};

/// Creates an escape rule.
/// The token is the initial rule to begin,
/// and then you can add rules that match after it.
template <typename EscapeToken>
LEXY_CONSTEVAL auto escape(EscapeToken)
{
    static_assert(lexy::is_token<EscapeToken>);
    return _escape<EscapeToken>{};
}

constexpr auto backslash_escape = escape(lit_c<'\\'>);
constexpr auto dollar_escape    = escape(lit_c<'$'>);
} // namespace lexyd

#endif // LEXY_DSL_DELIMITED_HPP_INCLUDED

