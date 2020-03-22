#if !defined(NODE_CPP)
#define NODE_CPP

#include "node.hpp"
using std::make_pair;

unordered_map<NodeType, int> Node::mapInlineType2IdStrSize = {
    make_pair(NodeType::bold, 2),
    make_pair(NodeType::italic, 1),
    make_pair(NodeType::codeSegment, 1),
    make_pair(NodeType::deleteLine, 2),
};
#endif // NODE_CPP
