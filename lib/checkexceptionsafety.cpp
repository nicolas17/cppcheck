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

//---------------------------------------------------------------------------
#include "checkexceptionsafety.h"
#include "symboldatabase.h"
#include "token.h"

//---------------------------------------------------------------------------

// Register CheckExceptionSafety..
namespace {
    CheckExceptionSafety instance;
}


//---------------------------------------------------------------------------

void CheckExceptionSafety::destructors()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    // Perform check..
    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        for (std::list<Function>::const_iterator j = i->functionList.begin(); j != i->functionList.end(); ++j) {
            // only looking for destructors
            if (j->type == Function::eDestructor && j->start) {
                // Inspect this destructor..
                for (const Token *tok = j->start->next(); tok != j->start->link(); tok = tok->next()) {
                    // throw found within a destructor
                    if (tok->str() == "throw") {
                        destructorsError(tok);
                        break;
                    }
                }
            }
        }
    }
}




void CheckExceptionSafety::deallocThrow()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    // Deallocate a global/member pointer and then throw exception
    // the pointer will be a dead pointer
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // only looking for delete now
        if (tok->str() != "delete")
            continue;

        // Check if this is something similar with: "delete p;"
        tok = tok->next();
        if (Token::simpleMatch(tok, "[ ]"))
            tok = tok->tokAt(2);
        if (!tok)
            break;
        if (!Token::Match(tok, "%var% ;"))
            continue;
        const unsigned int varid(tok->varId());
        if (varid == 0)
            continue;

        // we only look for global variables
        const Variable* var = symbolDatabase->getVariableFromVarId(varid);
        if (!var || !var->isGlobal())
            continue;

        // indentlevel..
        unsigned int indentlevel = 0;

        // Token where throw occurs
        const Token *ThrowToken = 0;

        // is there a throw after the deallocation?
        for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
            if (tok2->str() == "{")
                ++indentlevel;
            else if (tok2->str() == "}") {
                if (indentlevel == 0)
                    break;
                --indentlevel;
            }

            else if (tok2->str() == "throw")
                ThrowToken = tok2;

            // if the variable is not assigned after the throw then it
            // is assumed that it is not the intention that it is a dead pointer.
            else if (Token::Match(tok2, "%varid% =", varid)) {
                if (ThrowToken)
                    deallocThrowError(ThrowToken, tok->str());
                break;
            }
        }
    }
}

//---------------------------------------------------------------------------
//      catch(const exception & err)
//      {
//         throw err;            // <- should be just "throw;"
//      }
//---------------------------------------------------------------------------
void CheckExceptionSafety::checkRethrowCopy()
{
    if (!_settings->isEnabled("style"))
        return;

    const char catchPattern1[] = "catch (";
    const char catchPattern2[] = "%var% ) { %any%";

    const Token* tok = Token::findsimplematch(_tokenizer->tokens(), catchPattern1);
    while (tok) {
        const Token* endScopeTok = tok->next();
        const Token* endBracketTok = tok->next()->link();

        if (endBracketTok && Token::Match(endBracketTok->previous(), catchPattern2)) {
            const Token* startScopeTok = endBracketTok->next();
            endScopeTok = startScopeTok->link();
            const unsigned int varid = endBracketTok->previous()->varId();

            if (varid > 0) {
                const Token* rethrowTok = Token::findmatch(startScopeTok->next(), "throw %varid%", endScopeTok->previous(), varid);
                if (rethrowTok) {
                    rethrowCopyError(rethrowTok, endBracketTok->strAt(-1));
                }
            }
        }

        tok = Token::findsimplematch(endScopeTok->next(), catchPattern1);
    }
}

