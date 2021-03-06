/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NDEBUG

#include "tokenize.h"
#include "checkinternal.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestInternal : public TestFixture {
public:
    TestInternal() : TestFixture("TestInternal")
    { }

private:
    void run() {
        TEST_CASE(simplePatternInTokenMatch)
        TEST_CASE(complexPatternInTokenSimpleMatch)
        TEST_CASE(simplePatternSquareBrackets)
        TEST_CASE(simplePatternAlternatives)
        TEST_CASE(missingPercentCharacter);
    }

    void check(const std::string &code) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("internal");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code.c_str());
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Check..
        CheckInternal checkInternal;
        checkInternal.runSimplifiedChecks(&tokenizer, &settings, this);
    }

    void simplePatternInTokenMatch() {
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \";\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found simple pattern inside Token::Match() call: \";\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"%type%\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findmatch(tok, \";\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found simple pattern inside Token::findmatch() call: \";\"\n", errout.str());
    }

    void complexPatternInTokenSimpleMatch() {
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"%type%\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::simpleMatch() call: \"%type%\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findsimplematch(tok, \"%type%\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::findsimplematch() call: \"%type%\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findsimplematch(tok, \"} !!else\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::findsimplematch() call: \"} !!else\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findsimplematch(tok, \"foobar\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void simplePatternSquareBrackets() {
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"[\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"[ ]\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"[]\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::simpleMatch() call: \"[]\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"] [\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"] [ [abc]\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::simpleMatch() call: \"] [ [abc]\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"[.,;]\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::simpleMatch() call: \"[.,;]\"\n", errout.str());
    }

    void simplePatternAlternatives() {
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"||\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"|\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"a|b\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::simpleMatch() call: \"a|b\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"|= 0\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"| 0 )\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void missingPercentCharacter() {
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"%type%\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo %type% bar\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Missing % at the end of string
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"%type\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Missing percent end character in Token::Match() pattern: \"%type\"\n", errout.str());

        // Missing % in the middle of a pattern
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo %type bar\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Missing percent end character in Token::Match() pattern: \"foo %type bar\"\n", errout.str());

        // Bei quiet on single %
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo % %type% bar\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo % %type % bar\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Missing percent end character in Token::Match() pattern: \"foo % %type % bar\"\n", errout.str());

        // Find missing % also in 'alternatives' pattern
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo|%type|bar\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Missing percent end character in Token::Match() pattern: \"foo|%type|bar\"\n", errout.str());

        // Make sure we don't take %or% for a broken %oror%
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo|%oror%|bar\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestInternal)

#endif // #ifndef NDEBUG
