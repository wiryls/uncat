#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <uncat/far/far.hpp>
#include <uncat/rand/fill.hpp>

using uncat::far::operation, uncat::make_scanner, uncat::scan_mode;

TEST_CASE(R"(regex "[a-z]" to "_")", "[far]")
{
    auto input = std::string(512, '\0');
    uncat::rand::fill_with(input,
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789");

    auto const & pattern = R"([a-z])";
    auto const & replace = R"(_)";

    BENCHMARK("warm")
    {
        return input.size();
    };

    BENCHMARK(R"(std::regex)")
    {
        auto const r = std::regex(pattern);

        auto head = input.begin();
        auto tail = input.end();

        auto o = std::ostringstream();
        auto i = std::ostreambuf_iterator(o);
        std::regex_replace(i, head, tail, r, replace);
        return o.str();
    };

    BENCHMARK(R"(far::generator)")
    {
        auto o = std::ostringstream();
        auto f = make_scanner<scan_mode::regex>(pattern, replace);
        auto g = f.generate(input);
        for (auto s = g(); s.has_value(); s = g())
        {
            auto & v = s.value();
            if /**/ (auto insert = std::get_if<&operation::insert>(&v))
                o << std::string_view(*insert);
            else if (auto retain = std::get_if<&operation::retain>(&v))
                o << std::string_view(*retain);
        }
        return o.str();
    };

    BENCHMARK(R"(far::iterator)")
    {
        auto o = std::ostringstream();
        auto f = make_scanner<scan_mode::regex>(pattern, replace);
        for (auto & c : f.iterate(input))
        {
            if /**/ (auto insert = std::get_if<&operation::insert>(&c))
                o << std::string_view(*insert);
            else if (auto retain = std::get_if<&operation::retain>(&c))
                o << std::string_view(*retain);
        }
        return o.str();
    };

    BENCHMARK(R"(far::apply)")
    {
        auto const rule = uncat::far::rule<uncat::far::mode::regex, char>
            (pattern, replace, false);

        auto head = input.begin();
        auto tail = input.end();

        auto o = std::ostringstream();
        auto i = std::ostreambuf_iterator(o);

        uncat::far::apply(
            rule,
            [&](auto left, auto right) { o << std::string_view(left, right); },
            [ ](auto, auto) {},
            [&](auto left, auto right) { o << std::string_view(left, right); },
            head,
            tail);

        return o.str();
    };
}

TEST_CASE(R"(plain text "0011" to "1100")", "[far]")
{
    auto input = std::string(1024, '\0');
    uncat::rand::fill_with(input, "01");

    auto const & pattern = "0011";
    auto const & replace = "1100";

    BENCHMARK(R"(warm)")
    {
        return input.size();
    };

    BENCHMARK(R"(std::string.find)")
    {
        auto head = std::size_t{};
        auto tail = input.size();
        auto o = std::ostringstream();

        auto s = input.begin();
        while (head < tail)
        {
            auto i = input.find(pattern, head);
            if ( i == std::string::npos )
                break;

            o   << std::string_view(std::next(s, head), std::next(s, i))
                << replace;
            head = i + 1;
        }

        if (head != tail)
            o << std::string_view(std::next(s, head), std::next(s, tail));

        return o.str();
    };

    BENCHMARK(R"(far::generator)")
    {
        auto o = std::ostringstream();
        auto f = make_scanner<scan_mode::basic>(pattern, replace);
        auto g = f.generate(input);
        for (auto s = g(); s.has_value(); s = g())
        {
            auto & v = s.value();
            if /**/ (auto insert = std::get_if<&operation::insert>(&v))
                o << std::string_view(*insert);
            else if (auto retain = std::get_if<&operation::retain>(&v))
                o << std::string_view(*retain);
        }
        return o.str();
    };


    BENCHMARK(R"(far::iterator)")
    {
        auto o = std::ostringstream();
        auto f = make_scanner<scan_mode::basic>(pattern, replace);
        for (auto & c : f.iterate(input))
        {
            if /**/ (auto insert = std::get_if<&operation::insert>(&c))
                o << std::string_view(*insert);
            else if (auto retain = std::get_if<&operation::retain>(&c))
                o << std::string_view(*retain);
        }
        return o.str();
    };

    BENCHMARK(R"(far::apply)")
    {
        auto const rule = uncat::far::rule<uncat::far::mode::basic, char>(pattern, replace);

        auto head = input.begin();
        auto tail = input.end();

        auto o = std::ostringstream();
        auto i = std::ostreambuf_iterator(o);

        uncat::far::apply(
            rule,
            [&](auto left, auto right) { o << std::string_view(left, right); },
            [ ](auto, auto) {},
            [&](auto left, auto right) { o << std::string_view(left, right); },
            head,
            tail);

        return o.str();
    };
}
