#if !defined(NODE_HPP)
#define NODE_HPP

#include <list>
#include <vector>
#include <string>
#include <unordered_map>
using std::list;
using std::string;
using std::unordered_map;
using std::vector;

enum NodeType {
    title,
    orderedList,
    unorderedList,
    codeBlock,
    block,
    splitLine,
    codeSegment,
    bold,
    italic,
    deleteLine,
    link,
    image,
    text
};

class Node {
protected:
    list<Node *> children;
    Node *parent;
    NodeType type;
    // 正文段标签才有
    string textContent;
    int childCount;

public:
    // 指定行内标签标记字符串的长度，如加粗是**，长度为2
    static unordered_map<NodeType, int> mapInlineType2IdStrSize;

    Node(NodeType t = NodeType::text) : parent(nullptr), type(t), childCount(0) {}
    Node(const string &textContent) : type(NodeType::text), textContent(textContent), childCount(0) {}
    ~Node() {}
    void setParent(Node *p) { parent = p; }
    void addChild(Node *c) { children.push_back(c), c->setParent(this), childCount++; }
    void addChildFront(Node *c) { children.push_front(c), c->setParent(this), childCount++; }
    void setTextContent(const string &text) { textContent = text; }
    const auto getParent() { return parent; }
    const auto &getChildren() { return children; }
    const int getChildCount() { return childCount; }
    NodeType getType() { return type; }
};

class LinkImgNode : public Node {
private:
    string tagText, linkText;

public:
    LinkImgNode(NodeType t = NodeType::link) : Node(t) {}
    ~LinkImgNode() {}
    void setTagText(const string &tag) { tagText = tag; }
    void setLinkText(const string &link) { linkText = link; }
};
#endif // NODE_HPP
