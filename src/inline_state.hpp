#if !defined(INLINE_STATE_HPP)
#define INLINE_STATE_HPP

#include "Node.hpp"
#include <functional>
#include <exception>
#include <utility>
#include <iostream>
#include <algorithm>

using std::function;
using std::make_pair;
using std::pair;

class State {
public:
    typedef function<void(State *)> TypeNodeGotListener;
    typedef vector<TypeNodeGotListener> TypeListenerList;
    typedef function<bool(State *, const string &, const int)> TypeExtraGoNextChecker;

private:
    NodeType type;
    // 待完成的检查点
    int checkPointIndex;
    vector<int> checkPointStartPosList;

    TypeListenerList onNodeGotListeners;
    // 额外的检查函数 - 确定是否能够进入下一个检查点
    TypeExtraGoNextChecker extraGoNextChecker;

    const int maxCheckPoint;
    // 检查点的标识字符串
    vector<string> checkPointIdStrList;

public:
    State() : maxCheckPoint(0) {}
    State(NodeType t, const string &cpStr, int maxCP = 2)
        : type(t), checkPointIndex(0), maxCheckPoint(maxCP) {
        for (int i = 0; i < maxCP; i++) checkPointIdStrList.push_back(cpStr);
        checkPointStartPosList.resize(maxCP, 0);
        extraGoNextChecker = [](State *, const string &, const int) { return true; };
    }
    State(NodeType t, std::initializer_list<string> idList, int maxCP = 2)
        : type(t), maxCheckPoint(maxCP), checkPointIdStrList(idList) {
        if ((int)idList.size() != maxCP) throw std::runtime_error("checkPointIdStrList mismatch maxCheckPointCount");
        checkPointStartPosList.resize(maxCP, 0);
        extraGoNextChecker = [](State *, const string &, const int) { return true; };
    }
    ~State() {}
    void setExtraCanGoNextChecker(TypeExtraGoNextChecker e) { extraGoNextChecker = e; }
    void addOnNodeGotListener(TypeNodeGotListener listener) { onNodeGotListeners.push_back(listener); }
    void clear() {
        checkPointIndex = 0;
        std::fill(checkPointStartPosList.begin(), checkPointStartPosList.end(), 0);
    }
    bool canGoNextCP(const string &text, const int i) {
        const auto tmpStr = checkPointIdStrList[checkPointIndex];
        bool ret = (tmpStr == text.substr(i, tmpStr.size()));
        return ret && extraGoNextChecker(this, text, i);
    }
    // 完成当前检查点，进入下一阶段，i为当前检查点起始位置
    int gotoNextCP(const string &text, const int i) {
        const auto ret = i + checkPointIdStrList[checkPointIndex].size();
        checkPointStartPosList[checkPointIndex++] = i;
        if (checkPointIndex == maxCheckPoint)
            for (auto &&f : onNodeGotListeners) f(this);
        return ret;
    }
    // 获取节点开始位置
    int getStartPos() const { return checkPointStartPosList[0]; }

    // 获取节点结束位置，开区间
    int getEndPos() const { return checkPointStartPosList[maxCheckPoint - 1] + checkPointIdStrList[maxCheckPoint - 1].size(); }
    NodeType getNodeType() const { return type; }
    // 获取第x段的位置, 1<x<maxCheckPoint，返回 [start ,end)
    pair<int, int> getInterval(const int x) {
        if (x >= maxCheckPoint || x <= 0) return make_pair(0, 0);
        const int tmpStart = checkPointStartPosList[x - 1] + checkPointIdStrList[x - 1].size();
        const int tmpEnd = checkPointStartPosList[x];
        return make_pair(tmpStart, tmpEnd);
    }
    int getCheckPointIndex() const { return checkPointIndex; }
};

#endif // INLINE_STATE_HPP
