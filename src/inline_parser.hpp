#if !defined(INLINE_PARSER_HPP)
#define INLINE_PARSER_HPP

#include <deque>
#include <stack>
#include <unordered_map>
#include "test.hpp"
#include "Node.hpp"
#include "inline_state.hpp"

using std::deque;
using std::stack;
using std::unordered_map;

class InlineParser {
    friend class Tester;

public:
    InlineParser() {}
    ~InlineParser() {}
    // 节点指针，[起始，结束)
    struct TypeResult {
        Node *node;
        int start, end;
        TypeResult() : node(nullptr) {}
        TypeResult(Node *n, int s, int e) { node = n, start = s, end = e; }
        ~TypeResult() {}
        bool equal(const TypeResult &p) { return node == p.node && start == p.start && end == p.end; }
    };

    // 以树形结构返回所有节点，根节点无内容。返回空指针若text长度为0
    Node *parse(const string &text) {
        return makeTree(text, parseNodeSet(text));
    }

private:
    // 解析出所有的非纯文本节点
    vector<TypeResult> parseNodeSet(const string &text) {
        const int sz = text.size();
        if (sz <= 0) return {};
        // 类型列表
        vector<NodeType> typeList = {
            NodeType::bold, NodeType::italic, NodeType::deleteLine,
            NodeType::codeSegment, NodeType::image, NodeType::link};
        // 每个类型对应的状态
        unordered_map<NodeType, State> mapType2State = {
            make_pair(NodeType::bold, State(NodeType::bold, "**")),
            make_pair(NodeType::italic, State(NodeType::italic, "*")),
            make_pair(NodeType::deleteLine, State(NodeType::deleteLine, "~~")),
            make_pair(NodeType::codeSegment, State(NodeType::codeSegment, "`")),
            make_pair(NodeType::link, State(NodeType::link, {"[", "](", ")"}, 3)),
            make_pair(NodeType::image, State(NodeType::image, {"![", "](", ")"}, 3)),
        };
        //  斜体额外检查，斜体匹配必须是 [* + 非*]
        mapType2State[NodeType::italic].setExtraCanGoNextChecker([](State *state, const string &s, const int i) -> bool {
            return i + 1 >= (int)s.size() || s[i + 1] != '*';
        });
        vector<TypeResult> resultSet;
        // 确定一个普通节点后的监听器
        auto commonNodeGotListener = [&](State *state) {
            auto tmpNode = new Node(state->getNodeType());
            resultSet.push_back(TypeResult(tmpNode, state->getStartPos(), state->getEndPos()));
            const int targetStartPos = state->getStartPos();
            for (auto &&tmpType : typeList) {
                if (mapType2State.find(tmpType) != mapType2State.end()) {
                    auto &tmpState = mapType2State[tmpType];
                    if (tmpState.getCheckPointIndex() > 0 && tmpState.getStartPos() >= targetStartPos)
                        tmpState.clear();
                }
            }
            state->clear();
        };
        // 代码段、链接、图片的监听器
        auto csImgLinkNodeGotListener = [&](State *state) {
            // 删除不应该解析的
            if (!resultSet.empty()) {
                int i = resultSet.size() - 1;
                while (i >= 0 && resultSet[i].start >= state->getStartPos()) i--;
                resultSet.erase(resultSet.begin() + i + 1, resultSet.end());
            }
            // 添加节点，清空状态
            if (state->getNodeType() == NodeType::codeSegment) {
                auto tmpNode = new Node(state->getNodeType());
                resultSet.push_back(TypeResult(tmpNode, state->getStartPos(), state->getEndPos()));
            } else {
                const auto tmpNodeType = state->getNodeType();
                LinkImgNode *tmpNode = new LinkImgNode(tmpNodeType);
                auto tmpPair = state->getInterval(1);
                tmpNode->setTagText(text.substr(tmpPair.first, tmpPair.second - tmpPair.first));
                tmpPair = state->getInterval(2);
                tmpNode->setLinkText(text.substr(tmpPair.first, tmpPair.second - tmpPair.first));
                resultSet.push_back(TypeResult(tmpNode, state->getStartPos(), state->getEndPos()));
            }
            const int targetStartPos = state->getStartPos();
            for (auto &&tmpType : typeList) {
                if (mapType2State.find(tmpType) != mapType2State.end()) {
                    auto &tmpState = mapType2State[tmpType];
                    if (tmpState.getCheckPointIndex() > 0 && tmpState.getStartPos() >= targetStartPos)
                        tmpState.clear();
                }
            }
            state->clear();
        };
        // 添加监听器
        for (auto &&tmpType : typeList) {
            if (tmpType == NodeType::codeSegment || tmpType == NodeType::link || tmpType == NodeType::image)
                mapType2State[tmpType].addOnNodeGotListener(csImgLinkNodeGotListener);
            else
                mapType2State[tmpType].addOnNodeGotListener(commonNodeGotListener);
        }
        for (int i = 0; i < sz;) {
            // 确定是否自增i值
            bool doIncI = true;
            for (auto &&tmpType : typeList) {
                if (mapType2State[tmpType].canGoNextCP(text, i)) {
                    i = mapType2State[tmpType].gotoNextCP(text, i);
                    doIncI = false;
                    break;
                }
            }
            if (doIncI) i++;
        }
        return resultSet;
    }
    // 从结果集生成树形结构
    Node *makeTree(const string &text, const vector<TypeResult> &resultSet) {
        // FIXME 最大只支持2G文件
        const int sz = text.size();
        Node *root = new Node();
        deque<TypeResult> q;
        q.push_back(TypeResult(root, 0, sz));

        auto handleDirectChild = [](TypeResult p, const string &text) {
            // 只需考虑 加粗 斜体 代码段 删除线 这四个的子节点
            const NodeType type = p.node->getType();
            if (type == NodeType::link || type == NodeType::image)
                return;
            const int idSize = Node::mapInlineType2IdStrSize[type];
            const int s = p.start + idSize, e = p.end - idSize;
            p.node->addChild(new Node(text.substr(s, e - s)));
        };
        auto handleFaFirstChild = [](TypeResult fa, TypeResult child, const string &text) {
            const NodeType faType = fa.node->getType();
            const int szFaIdStr = Node::mapInlineType2IdStrSize[faType];
            if (fa.start + szFaIdStr < child.start) {
                const int s = fa.start + szFaIdStr, e = child.start;
                fa.node->addChildFront(new Node(text.substr(s, e - s)));
            }
        };
        auto handleFaLastChild = [](TypeResult fa, TypeResult child, const string &text) {
            const int szFaIdStr = Node::mapInlineType2IdStrSize[fa.node->getType()];
            if (child.end < fa.end - szFaIdStr)
                fa.node->addChildFront(new Node(text.substr(child.end, fa.end - szFaIdStr - child.end)));
        };
        auto handleSibling = [](TypeResult fa, TypeResult p, TypeResult child, const string &text) {
            if (p.end < child.start)
                fa.node->addChildFront(new Node(text.substr(p.end, child.start - p.end)));
        };

        for (int i = resultSet.size() - 1; i >= 0; i--) {
            TypeResult tmpRes = resultSet[i];
            TypeResult lastTop = q.back();
            while (q.back().start >= tmpRes.end && !q.empty()) {
                lastTop = q.back();
                // q.back的直接纯文本子节点
                if (lastTop.node->getChildCount() == 0)
                    handleDirectChild(lastTop, text);
                q.pop_back();
                // q.back 父，lastTop 为第一个子节点
                handleFaFirstChild(q.back(), lastTop, text);
            }
            // q.back() 父，tmpRes最后一个子节点
            if (lastTop.equal(q.back())) {
                handleFaLastChild(q.back(), tmpRes, text);
            }
            // q.back 父，LasTop 和 tmpRes 兄弟
            else {
                handleSibling(q.back(), tmpRes, lastTop, text);
            }
            q.back().node->addChildFront(tmpRes.node);
            q.push_back(tmpRes);
        }
        TypeResult lastTop;
        while (!q.empty()) {
            lastTop = q.back();
            q.pop_back();
            if (lastTop.node->getChildCount() == 0) handleDirectChild(lastTop, text);
            if (!q.empty()) handleFaFirstChild(q.back(), lastTop, text);
        }
        return root;
    }
};

#endif // INLINE_PARSER_HPP
