#ifndef RANGETREE_HPP
#define RANGETREE_HPP

#include <limits>
#include <memory>
#include <numeric>
#include <stack>
#include <tuple>

namespace xutil
{
template<typename K, typename V>
class RangeTree
{
public:
    void insert(K key, V value)
    {
        auto left = std::numeric_limits<K>::min();
        auto right = std::numeric_limits<K>::max();
        auto *cur = &node;
        while(true)
        {
            cur->sum += value;
            if(left >= right)
            {
                break;
            }
            const auto middle = std::midpoint(left, right);
            if(key <= middle)
            {
                right = middle;
                if(!cur->left)
                {
                    cur->left = std::make_shared<Node>();
                }
                cur = cur->left.get();
            }
            else
            {
                left = middle + 1;
                if(!cur->right)
                {
                    cur->right = std::make_shared<Node>();
                }
                cur = cur->right.get();
            }
        }
    }

    [[nodiscard]] V querySum(K begin, K end) const
    {
        std::stack<std::tuple<V, V, const Node *>> ranges;
        ranges.emplace(std::numeric_limits<V>::min(),
                       std::numeric_limits<V>::max(), &node);
        V sum = 0;
        while(!ranges.empty())
        {
            const auto [rangeLeft, rangeRight, cur] = ranges.top();
            ranges.pop();
            if(begin <= rangeLeft && end >= rangeRight)
            {
                sum += cur->sum;
            }
            else
            {
                const auto middle = std::midpoint(rangeLeft, rangeRight);
                if(begin <= middle && cur->left)
                {
                    ranges.emplace(rangeLeft, middle, cur->left.get());
                }
                if(end > middle && cur->right)
                {
                    ranges.emplace(middle + 1, rangeRight, cur->right.get());
                }
            }
        }
        return sum;
    }

private:
    struct Node
    {
        std::shared_ptr<Node> left{};
        std::shared_ptr<Node> right{};
        V sum{};
    };

    Node node;
};
}

#endif
