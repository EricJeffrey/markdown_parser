#if !defined(TEST_CPP)
#define TEST_CPP

#include "test.hpp"
#include "inline_parser.hpp"

/*
fdslk**sdlkf~~kdsdf*sdlkf*fjsdf~~sdfsdf**
fdslk**sdlfk**~~sdf*sdfk*sdf~~sdfgd
fdslk*dsklg~~sdlkg~~`*sdg`~~sdklg~~sdf~~d~~
*/
void Tester::testInlineParse() {
    string text = "f![s*dsf~~dsf~~sdf**sdflk**ds*d](sdf.co)dssd[Sf](SDf)dflk*dsklg~~sdlkg~~`xx*sd`g`~~sdklg~~sdf~~d~~";
    text = "hello";
    text = "*sdf**sdf**sdf*";
    InlineParser parser;
    auto res = parser.parseNodeSet(text);
    auto root = parser.makeTree(text, res);
    return;
}

#endif // TEST_CPP
