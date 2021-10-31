#pragma once

/// Note:
/// Some code in this file is useless but still reserved. I leave it here in
/// order to remind me not to over-design.
///
/// Also:
/// this file is base on a RUST version:
/// https://github.com/wiryls/far/blob/deprecated/gtk4-rs/src/far/diff.rs and
/// https://github.com/wiryls/far/blob/deprecated/gtk4-rs/src/far/faregex.rs

#include <type_traits>
#include <concepts>
#include <iterator>
#include <functional>
#include <execution>
#include <ranges>
#include <memory>
#include <optional>
#include <variant>
#include <string>
#include <string_view>
#include <regex>

namespace uncat { namespace far
{
    // char types that scanner supports.
    template<typename C>
    concept char_type = requires
    {
        typename std::regex_traits<C>::char_type;
        typename std:: char_traits<C>::char_type;
    };

    // iterators of char_type.
    template<typename I, typename C>
    concept iterative
        = char_type<C>
       && std::input_or_output_iterator<I>
       && std::same_as<std::iter_value_t<I>, C>
        ;

    // forward iterators of char_type.
    // - required by std::regex constructor.
    template<typename I, typename C>
    concept forward_iterative
        = iterative<I, C>
       && std::forward_iterator<I>
        ;

    // bidirectional iterators of char_type.
    // - required by std::default_searcher.
    template<typename I, typename C>
    concept bidirectional_iterative
        = iterative<I, C>
       && std::bidirectional_iterator<I>
        ;

    // ranges of char_type.
    template<typename S, typename C>
    concept sequence
        = std::ranges::range<S>
       && iterative<std::ranges::iterator_t<S>, C>
        ;

    // forward ranges of char_type.
    // - used when constructing scanner.
    template<typename S, typename C>
    concept forward_sequence
        = std::ranges::range<S>
       && forward_iterative<std::ranges::iterator_t<S>, C>
        ;

    // bidirectional ranges of char_type.
    // - used when constructing scanner.
    template<typename S, typename C>
    concept bidirectional_sequence
        = std::ranges::range<S>
       && bidirectional_iterative<std::ranges::iterator_t<S>, C>
        ;
}}

namespace uncat { namespace far { namespace aux
{
    namespace detail
    {
        //// rewite std::ranges::end("string literal") to that - 1
        struct end
        {
        private:
            template<std::ranges::range R>
            struct zero_suffixed : std::false_type {};

            template<typename C, std::size_t N>
            requires (N > 0) &&
            requires (C const (&c)[N])
            {
                { c + N } -> std::contiguous_iterator;
            }
            struct zero_suffixed<C(&)[N]> : std::true_type
            {
                static constexpr std::ptrdiff_t N = N;
            };

        public:
            template<std::ranges::range R>
            auto constexpr operator()(R && r) const
            noexcept(noexcept(std::ranges::end(std::forward<R>(r))))
            {
                if constexpr (zero_suffixed<R>::value)
                    // or maybe return a custom sentinel?
                    // see: https://www.foonathan.net/2020/03/iterator-sentinel/
                    return std::forward<R>(r) + zero_suffixed<R>::N - 1;
                else
                    return std::ranges::end(std::forward<R>(r));
            }
        };
    }

    inline constexpr auto begin = std::ranges::begin;
    inline constexpr auto end   = detail::end{};
}}}

namespace uncat { namespace far
{
    //// helpers

    template<char_type C>
    struct icase_comparator
    {
        std::locale locale;
        auto operator()(C lhs, C rhs) const noexcept -> bool
        {
            return std::tolower(lhs, locale) == std::tolower(rhs, locale);
        }
    };

    template<char_type C, std::invocable<C, C> F = std::equal_to<>>
    struct basic_or_icase_rule
    {
    public: // types
        using string          = std::basic_string<C>;
        using string_iterator = typename string::const_iterator;
        using prefered        = std::boyer_moore_searcher<string_iterator, std::hash<C>, F>;
        using fallback        = std::    default_searcher<string_iterator, F>;
        using switcher        = std::pair<prefered, fallback>;

    public: // data
        string   pattern;
        string   replace;
        switcher chooser;

    public: // constructors and assignments
        basic_or_icase_rule(sequence<C> auto const & p, sequence<C> auto const & r)
            : pattern(aux::begin(p), aux::end(p))
            , replace(aux::begin(r), aux::end(r))
            , chooser(bind(pattern))
        {}
        basic_or_icase_rule(basic_or_icase_rule && rhs)
            : pattern(std::move(rhs.pattern))
            , replace(std::move(rhs.replace))
            , chooser(bind(pattern))
        {}
        basic_or_icase_rule(basic_or_icase_rule const & rhs)
            : pattern(rhs.pattern)
            , replace(rhs.replace)
            , chooser(bind(pattern))
        {}
        auto operator=(basic_or_icase_rule && rhs) -> basic_or_icase_rule &
        {
            pattern = std::move(rhs.pattern);
            replace = std::move(rhs.replace);
            chooser = bind(pattern);
            return this;
        }
        auto operator=(basic_or_icase_rule const & rhs) -> basic_or_icase_rule &
        {
            pattern = rhs.pattern;
            replace = rhs.replace;
            chooser = bind(pattern);
            return this;
        }

    private:
        auto static inline bind(string const & that) -> switcher
        {
            // note: searcher may have a reference to pattern, so I store pattern
            // as a member. Be careful that do not change pattern. Any reallocation
            // may break our searcher, as well as move a small string.
            return std::make_pair
                ( prefered(that.begin(), that.end(), std::hash<C>{}, F{})
                , fallback(that.begin(), that.end(), F{}) );
        }
    };

    template<typename T, typename I>
    struct operator_string_view {};

    template<typename T, std::contiguous_iterator I>
    requires char_type<std::iter_value_t<I>>
    struct operator_string_view<T, I>
    {
        explicit constexpr operator auto() const noexcept
        {
            auto & self = *static_cast<T const *>(this);
            return std::basic_string_view(self.first, self.second);
        }
    };

    template<std::input_or_output_iterator I>
    struct iterator_pair
        : std::pair<I, I>
        , operator_string_view<iterator_pair<I>, I>
    {
        using std::pair<I, I>::pair;

        decltype(auto) constexpr begin()       noexcept { return this-> first; }
        decltype(auto) constexpr begin() const noexcept { return this-> first; }
        decltype(auto) constexpr   end()       noexcept { return this->second; }
        decltype(auto) constexpr   end() const noexcept { return this->second; }

        // Not sure why msvc forbids me using "requires" at the the end of
        // "operator auto()" after an update. It forces me to replace "requires"
        // with CRTP.
    };
}}

namespace uncat { namespace far
{
    //// rule (immutable)

    enum struct mode
    {
        basic,
        icase,
        regex,
    };

    template<mode M, char_type C>
    struct rule;

    template<char_type C>
    struct rule<mode::basic, C> : basic_or_icase_rule<C>
    {
        rule(sequence<C> auto const & pattern, sequence<C> auto const & replace)
            : basic_or_icase_rule<C>(pattern, replace)
        {}
    };

    template<char_type C>
    struct rule<mode::icase, C> : basic_or_icase_rule<C, icase_comparator<C>>
    {
        rule(sequence<C> auto const & pattern, sequence<C> auto const & replace)
            : basic_or_icase_rule<C, icase_comparator<C>>(pattern, replace)
        {}
    };

    template<char_type C>
    struct rule<mode::regex, C>
    {
        template<forward_sequence<C> P, sequence<C> R>
        rule(P const & pattern, R const & replace, bool ignore_case)
            : pattern()
            , replace(aux::begin(replace), aux::end(replace))
            , error()
        {
            try
            {
                using option_type = std::regex_constants::syntax_option_type;
                auto constexpr basic = std::regex_constants::ECMAScript;
                auto constexpr icase = std::regex_constants::icase;

                auto option = static_cast<option_type>(basic | (ignore_case ? icase : 0));
                this->pattern = std::basic_regex<C>
                    ( aux::begin(pattern)
                    , aux::end(pattern)
                    , option );
            }
            catch (std::regex_error const & ex)
            {
                error = ex.code();
            }
        }

        std::basic_regex <C> pattern;
        std::basic_string<C> replace;
        std::optional<std::regex_constants::error_type> error;
    };

    //// change (output)

    enum struct operation : int
    {
        retain = 0,
        remove = 1,
        insert = 2,
    };

    template<typename T>
    requires std::same_as<operation, std::remove_cvref_t<T>> && (!std::is_reference_v<T>)
    auto inline consteval operator&(T && t) noexcept
    {
        // let us treat std::underlying_type_t as the "address" of our enum.
        return static_cast<std::underlying_type_t<operation>>(t);
    }

    template<char_type C, bidirectional_iterative<C> I>
    struct retain : iterator_pair<I>
    {
        using iterator = I;
        using iterator_pair<I>::iterator_pair;
    };

    template<char_type C, bidirectional_iterative<C> I>
    struct remove : iterator_pair<I>
    {
        using iterator = I;
        using iterator_pair<I>::iterator_pair;
    };

    template<char_type C>
    struct insert : iterator_pair<typename std::basic_string<C>::const_iterator>
    {
        using iterator = typename std::basic_string<C>::const_iterator;
        using iterator_pair<iterator>::iterator_pair;
    };

    template<char_type C, std::bidirectional_iterator I>
    using change = std::variant
        < retain<C, I>
        , remove<C, I>
        , insert<C> >;
}}

namespace uncat { namespace far
{
    //// generator

    enum struct generator_status
    {
        stopped,
        startup,
        after_retain,
        after_remove,
        after_insert,
        before_stopped,
    };

    template<mode M, char_type C, bidirectional_iterative<C> I>
    class generator;

    // attention:
    // lifetime of rule must be longer than this generator.
    template<mode M, char_type C, bidirectional_iterative<C> I>
    generator(rule<M, C> const &, I, I) -> generator<M, C, I>;

    template<mode M, char_type C, bidirectional_iterative<C> I>
    requires (M == mode::basic || M == mode::icase)
    class generator<M, C, I>
    {
    public:
        auto operator()() -> std::optional<change<C, I>>
        {
            // note: because coroutines from C++20 are hard to use and have
            // poor performance (in contrast to for-loop), I choose duff's
            // device.
            // https://stackoverflow.com/questions/57726401
            // https://www.reddit.com/r/cpp/comments/gqi0io

            switch (stat)
            {
            case generator_status::startup:

                while (head != tail)
                {
                    using tag = std::iterator_traits<I>::iterator_category;
                    if constexpr (std::is_base_of_v<std::random_access_iterator_tag, tag>)
                        curr = (rule.chooser. first)(head, tail);
                    else
                        curr = (rule.chooser.second)(head, tail);

                    if (curr.first == curr.second)
                        break;

                    if (head != curr.first)
                    {
                        stat = generator_status::after_retain;
                        return retain<C, I>(head, curr.first);
                    }

            [[fallthrough]];
            case generator_status::after_retain:

                    {
                        stat = generator_status::after_remove;
                        return remove<C, I>(curr.first, curr.second);
                    }

            [[fallthrough]];
            case generator_status::after_remove:

                    if (!rule.replace.empty())
                    {
                        stat = generator_status::after_insert;
                        return insert<C>(rule.replace.begin(), rule.replace.end());
                    }

            [[fallthrough]];
            case generator_status::after_insert:

                    head = curr.second;
                }

            [[fallthrough]];
            case generator_status::before_stopped:

                if (head != tail)
                {
                    stat = generator_status::stopped;
                    return retain<C, I>(head, tail);
                }

            [[fallthrough]];
            case generator_status::stopped:
            default:

                return std::nullopt;
            }
        }

    public:
        generator(rule<M, C> const & rule, I first, I last) noexcept
            : rule(rule)
            , head(first)
            , tail(last)
            , curr{}
            , stat
                { first == last
                ? generator_status::stopped
                : rule.pattern == rule.replace
                ? generator_status::before_stopped
                : generator_status::startup }
        {
            /**/ if (first == last)
                stat = generator_status::stopped;
            else if (rule.pattern == rule.replace)
                stat = generator_status::before_stopped;
        }

    private:
        using iterator = I;

        rule<M, C> const &            rule;
        iterator                      head;
        iterator                      tail;
        std::pair<iterator, iterator> curr;
        generator_status              stat;
    };

    template<char_type C, bidirectional_iterative<C> I>
    class generator<mode::regex, C, I>
    {
    public:
        auto operator()() -> std::optional<change<C, I>>
        {
            switch (stat)
            {
            case generator_status::startup:

                for (; head != tail; ++ head)
                {
                    if (head->empty()) [[unlikely]]
                        continue;

                    buff.clear();
                    head->format(std::back_inserter(buff), rule.replace);

                    {
                        auto constexpr e = std::execution::unseq;
                        auto const &   m = (*head)[0];
                        auto const &   b = buff;

                        if (m.first == m.second ||
                            std::equal(e, b.begin(), b.end(), m.first, m.second))
                            [[unlikely]]
                            continue;

                        if (curr != m.first)
                        {
                            stat = generator_status::after_retain;
                            return retain<C, I>(curr, m.first);
                        }
                    }

            [[fallthrough]];
            case generator_status::after_retain:

                    {
                        auto const & match = (*head)[0];
                        stat = generator_status::after_remove;
                        return remove<C, I>(match.first, match.second);
                    }

            [[fallthrough]];
            case generator_status::after_remove:

                    if (!buff.empty())
                    {
                        stat = generator_status::after_insert;
                        return insert<C>(buff.begin(), buff.end());
                    }

            [[fallthrough]];
            case generator_status::after_insert:

                    curr = (*head)[0].second;
                }

                stat = generator_status::stopped;
                if (curr != last)
                    return retain<C, I>(curr, last);

            [[fallthrough]];
            case generator_status::stopped:
            default:

                return std::nullopt;
            }
        }

    public:
        generator(rule<mode::regex, C> const & rule, I first, I last)
            : rule(rule)
            , curr(first)
            , last(last)
            , head()
        {
            if (first == last)
                stat = generator_status::stopped;
            else
                head = std::regex_iterator<I>(first, last, rule.pattern);
        }

    private:
        using iterator = I;

        rule<mode::regex, C> const &  rule;
        iterator                      curr;
        iterator                      last;
        std::regex_iterator<iterator> head;
        std::regex_iterator<iterator> tail{ std::regex_iterator<iterator>() };
        std::basic_string<C>          buff{};
        generator_status              stat{ generator_status::startup };
    };

    //// iterator

    template<mode M, char_type C, bidirectional_iterative<C> I>
    class iterator;

    template<mode M, char_type C, bidirectional_iterative<C> I>
    iterator(generator<M, C, I> &&) -> iterator<M, C, I>;

    template<mode M, char_type C, bidirectional_iterative<C> I>
    class iterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = change<C, I>;
        using pointer           = change<C, I> const *;
        using reference         = change<C, I> const &;

    public:
        auto operator*() const -> reference
        {
            return last.value();
        }

        auto operator->() const noexcept -> pointer
        {
            return last.has_value() ? &last.value() : nullptr;
        }

        auto operator++() noexcept -> iterator &
        {
            if (func == nullptr) [[unlikely]]
                return *this;

            // generate last
            // generate next
            // if last can merge with next (#3)
            //    merge last with next
            //    generate next
            //    retry #3

            auto & generate = *func.get();

            if (!last.has_value()) [[unlikely]]
            {
                last = generate();
            }
            else
            {
                last = std::move(next);
            }

            if (!last.has_value()) [[unlikely]]
            {
                func = nullptr;
                return *this;
            }

            auto & lhs = last.value();
            auto index = lhs.index();
            for (;;)
            {
                if (next = generate(); !next.has_value())
                    break;

                auto & rhs = next.value();
                if (index != rhs.index())
                    break;

                using op = operation;
                switch (index)
                {
                case &op::retain:
                {
                    auto l = std::get_if<&op::retain>(&lhs);
                    auto r = std::get_if<&op::retain>(&rhs);
                    if (l->second != r->first)
                        return *this;

                    l->second = r->second;
                    break;
                }
                case &op::remove:
                {
                    auto l = std::get_if<&op::remove>(&lhs);
                    auto r = std::get_if<&op::remove>(&rhs);
                    if (l->second != r->first)
                        return *this;

                    l->second = r->second;
                    break;
                }
                case &op::insert:
                {
                    // not a good idea to expand std::basic_string_view
                    return *this;
                }
                default: [[unlikely]]
                {
                    // should never arrive here, or maybe throw ?
                    last = std::nullopt;
                    next = std::nullopt;
                    func = nullptr;
                    return *this;
                }
                }
            }

            return *this;
        }

        auto operator++(int) noexcept -> iterator
        {
            auto iter = *this;
            ++ *this;
            return iter;
        }

    public:
        friend auto operator==(iterator const & lhs, iterator const & rhs) -> bool
        {
            auto & l = lhs.func;
            auto & r = rhs.func;
            return (l != nullptr && r != nullptr && l.get() == r.get())
                || (l == nullptr && r == nullptr)
                ;
        }

        friend auto operator!=(iterator const & lhs, iterator const & rhs) -> bool
        {
            return !(lhs == rhs);
        }

    public:
        explicit iterator(generator<M, C, I> && gen) noexcept
            : func(std::make_shared<generator<M, C, I>>(std::move(gen)))
        {
            ++ *this;
        }

        iterator() noexcept
            : func()
        {}

    private:
        // as iterator is suggested to be copyable, we use std::shared_ptr
        // to make a generator be owned by its iterators.
        using shared_generator = std::shared_ptr<generator<M, C, I>>;

        shared_generator          func;
        std::optional<value_type> next{ std::nullopt };
        std::optional<value_type> last{ std::nullopt };
    };
}}

namespace uncat { namespace far
{
    //// control group

    template<typename F, typename I>
    concept output = requires (F fun, I first, I last)
    {
        { fun(first, last) } -> std::same_as<void>;
    };

    template
        < mode M
        , char_type C
        , bidirectional_iterative<C> I
        , output<I> R /* retain */
        , output<I> O /* remove */
        , output<typename std::basic_string<C>::const_iterator> N /* insert */
        > requires (M == mode::basic || M == mode::icase)
    auto apply
        ( rule<M, C> const & rule
        , R retain
        , O remove
        , N insert
        , I head
        , I tail
        ) -> void
    {
        using tag = std::iterator_traits<I>::iterator_category;
        auto const add = !rule.replace.empty();
        auto match = std::pair<I, I>{};
        while (head != tail)
        {
            if constexpr (std::is_base_of_v<std::random_access_iterator_tag, tag>)
                match = (rule.chooser.first )(head, tail);
            else
                match = (rule.chooser.second)(head, tail);

            if (match.first == match.second)
                break;

            if (head != match.first)
                retain(head, match.first);

            remove(match.first, match.second);

            if (add)
                insert(rule.replace.begin(), rule.replace.end());

            head = match.second;
        }

        if (head != tail)
            retain(head, tail);
    }

    template
        < mode M
        , char_type C
        , bidirectional_iterative<C> I
        , output<I> R /* retain */
        , output<I> O /* remove */
        , output<typename std::basic_string<C>::const_iterator> N /* insert */
        > requires (M == mode::regex)
    auto apply
        ( rule<M, C> const & rule
        , R retain
        , O remove
        , N insert
        , I first
        , I last
        ) -> void
    {
        auto constexpr static exe = std::execution::unseq;
        auto head = std::regex_iterator<I>(first, last, rule.pattern);
        auto tail = std::regex_iterator<I>();
        auto buff = std::basic_string  <C>();

        for (; head != tail; ++ head)
        {
            if (head->empty()) [[unlikely]]
                continue;

            buff.clear();
            head->format(std::back_inserter(buff), rule.replace);

            auto const & [_, left, right] = (*head)[0];
            if (left == right ||
                std::equal(exe, buff.begin(), buff.end(), left, right)) [[unlikely]]
                continue;

            if (first != left)
                retain(first, left);

            remove(left, right);

            if (!buff.empty())
                insert(buff.begin(), buff.end());

            first = right;
        }

        if (first != last)
            retain(first, last);
    }
}}

namespace uncat
{
    //// export to outter namespace

    using scan_mode = far::mode;

    template<far::mode M, far::char_type C>
    class scanner
    {
    public:
        template<far::bidirectional_sequence<C> R>
        [[nodiscard]] auto inline generate(R & container) const
        {
            using namespace far;
            return generate(aux::begin(container), aux::end(container));
        }

        template<far::bidirectional_iterative<C> I>
        [[nodiscard]] auto inline generate(I first, I last) const
            -> far::generator<M, C, I>
        {
            return far::generator<M, C, I>(rule, std::move(first), std::move(last));
        }

    public:
        template<far::bidirectional_sequence<C> R>
        [[nodiscard]] auto inline iterate(R & container) const
        {
            using namespace far;
            return iterate(aux::begin(container), aux::end(container));
        }

        template<far::bidirectional_iterative<C> I>
        [[nodiscard]] auto inline iterate(I first, I last) const
        {
            return far::iterator_pair<far::iterator<M, C, I>>
                { far::iterator<M, C, I>(generate(std::move(first), std::move(last)))
                , far::iterator<M, C, I>() };
        }

    public:
        using mode = far::mode;

        [[nodiscard]] operator bool() const
        requires (M == mode::regex)
        {
            return !rule.error.has_value();
        }

    public:
        template<far::sequence<C> P, far::sequence<C> R>
        requires (M == mode::basic || M == mode::icase)
        scanner(P const & pattern, R const & replace)
            : rule(pattern, replace)
        {}

        template<far::sequence<C> P, far::sequence<C> R>
        requires (M == mode::regex)
        scanner(P const & pattern, R const & replace, bool ignore_case = false)
            : rule(pattern, replace, ignore_case)
        {}

    private:
        far::rule<M, C> rule;
    };

    template
        < far::mode M
        , std::ranges::forward_range P
        , std::ranges::forward_range R
        , far::char_type C = std::ranges::range_value_t<P>
        > requires (M == far::mode::basic || M == far::mode::icase)
    [[nodiscard]] auto inline make_scanner
        ( P const & pattern
        , R const & replace )
    {
        return scanner<M, C>(pattern, replace);
    }

    template
        < far::mode M
        , std::ranges::forward_range P
        , std::ranges::forward_range R
        , far::char_type C = std::ranges::range_value_t<P>
        > requires (M == far::mode::regex)
    [[nodiscard]] auto inline make_scanner
        ( P const & pattern
        , R const & replace
        , bool ignore_case = false )
    {
        return scanner<M, C>(pattern, replace, ignore_case);
    }
}
