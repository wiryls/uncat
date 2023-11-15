#include <cstddef>
#include <iostream>
#include <ranges>
#include <tuple>
#include <utility>
#include <vector>

struct ListNode
{
    int        val;
    ListNode * next;

    ListNode()
        : val(0)
        , next(nullptr)
    {}

    explicit ListNode(int x)
        : val(x)
        , next(nullptr)
    {}

    ListNode(int x, ListNode * next)
        : val(x)
        , next(next)
    {}
};

struct Solution
{
    static ListNode * reverseKGroup(ListNode * head, int k)
    {
        auto   dummy = ListNode(0, head);
        auto * prev  = &dummy;
        auto * node  = prev->next;
        while (node != nullptr)
        {
            auto [next, done] = step(node, k);
            if (!done)
                break;

            prev->next = reverse(node, next);
            prev       = node;
            node       = next;
        }
        return dummy.next;
    }

    static std::pair<ListNode *, bool> step(ListNode * node, int k)
    {
        while (node != nullptr && k > 0)
        {
            --k;
            node = node->next;
        }
        return {node, k == 0};
    }

    static ListNode * reverse(ListNode * node, ListNode * tail)
    {
        auto * prev = tail;
        while (node != tail)
        {
            auto * next = node->next;
            node->next  = prev;
            prev        = node;
            node        = next;
        }
        return prev;
    }
};

int main()
{
    auto build_list = [](std::vector<int> const & list)
    {
        auto output = std::vector<ListNode>(list.size());
        if (!list.empty())
            output[0].val = list[0];

        for (auto i : std::views::iota(std::size_t(1), list.size()))
        {
            output[i].val      = list[i];
            output[i - 1].next = &output[i];
        }
        return output;
    };

    auto collect = [](ListNode * node)
    {
        auto output = std::vector<int>();
        while (node != nullptr)
        {
            output.push_back(node->val);
            node = node->next;
        }
        return output;
    };

    auto cases = std::vector<std::tuple<std::vector<int>, int, std::vector<int>>>{
        // NOLINT(cppcoreguidlines-avoid-magic-numbers)
        {{1, 2, 3, 4, 5}, 2, {2, 1, 4, 3, 5}},
        {{1, 2, 3, 4, 5}, 3, {3, 2, 1, 4, 5}},
    };

    for (auto const & [list, k, expected] : cases)
    {
        auto   solution = Solution();
        auto   input    = build_list(list);
        auto * root     = input.empty() ? nullptr : &input.front();

        root = solution.reverseKGroup(root, k);
        if (expected != collect(root))
        {
            std::cout << "tests fail" << '\n';
            return 0;
        }
    }

    std::cout << "done" << '\n';
    return 0;
}
