// This class is used for parsing simple text math expressions

using namespace std;

#include <ctype.h>
#include <iostream>

#include "ExpressionParser.h"
#include "Genome.h"

const char *g_ExpressionParserFunctionList[] =
{
    "if"
};
const int g_ExpressionParserFunctionListArgs[] =
{
    3
};
const int g_ExpressionParserFunctionListSize = 1;

ExpressionParser::ExpressionParser()
{
}


ExpressionParser::~ExpressionParser()
{
    unsigned int i;
    for (i = 0; i < m_ExpressionParserTokenList.size(); i++)
        delete m_ExpressionParserTokenList[i];
}


// initialise the ExpressionParser based on a string
// returns 0 if successful
// the character input[length] must exist (can be '\0')
int ExpressionParser::CreateFromString(char *input, int length)
{
    int index = 0;
    int parenthesisStart;
    int parenthesisCount;
    int functionStart;
    ExpressionParserToken *expressionParserToken;
    bool finished;
    char byte;
    char *ptr;
    int i, j;
    bool expectingNumber = true;
    ExpressionParser *expressionParser;
    int nargs;

    if (m_ExpressionParserTokenList.size() > 0)
    {
        unsigned int ii;
        for (ii = 0; ii < m_ExpressionParserTokenList.size(); ii++)
            delete m_ExpressionParserTokenList[ii];
        m_ExpressionParserTokenList.clear();
    }
    
    while (index < length)
    {
        if (isspace(input[index]) == 0) // not white space
        {
            if (input[index] == '(') // left parenthesis - need to find matching ')' and recurse
            {
                if (expectingNumber == false)
                {
                    cerr << "ParseError: Not expecting a number\n" << input << "\n";
                    return 4;
                }
                expectingNumber = false;
                finished = false;
                index++;
                parenthesisStart = index;
                parenthesisCount = 1;
                while (finished == false)
                {
                    if (input[index] == '(') parenthesisCount++;
                    else if (input[index] == ')') parenthesisCount--;
                    if (parenthesisCount == 0)
                    {
                        expressionParserToken = new ExpressionParserToken();
                        m_ExpressionParserTokenList.push_back(expressionParserToken);
                        expressionParserToken->type = EPParser;
                        expressionParser = new ExpressionParser();
                        expressionParserToken->parserList.push_back(expressionParser);
                        expressionParser->CreateFromString(&input[parenthesisStart], index - parenthesisStart);
                        finished = true;
                    }
                    index++;
                    if (index >= length && finished == false)
                    {
                        cerr << "Parse Error: Unmatched '('\n" << input << "\n";
                        return 1;
                    }
                }
            }
            
            else if (isalpha(input[index])) // variable or function
            {
                if (expectingNumber == false)
                {
                    cerr << "ParseError: Not expecting a number\n" << input << "\n";
                    return 4;
                }
                expectingNumber = false;
                finished = false;
                expressionParserToken = new ExpressionParserToken();
                m_ExpressionParserTokenList.push_back(expressionParserToken);
                expressionParserToken->type = EPVariable;
                expressionParserToken->name = input[index];
                while (finished == false)
                {
                    index++;
                    if (index >= length) finished = true;
                    else if (isalnum(input[index]) == 0) finished = true;
                    else expressionParserToken->name += input[index];
                }
                nargs = 1; // value if array
                for (i = 0; i < g_ExpressionParserFunctionListSize; i++)
                {
                    if (expressionParserToken->name == g_ExpressionParserFunctionList[i])
                    {
                        expressionParserToken->type = EPFunction;
                        expressionParserToken->index = i;
                        nargs = g_ExpressionParserFunctionListArgs[i];
                        break;
                    }
                }
                if (index >= length)
                {
                    cerr << "ParseError: Cannot find '('\n" << input << "\n";
                    return 5;
                }
                if (input[index] != '(')
                {
                    cerr << "ParseError: Cannot find '('\n" << input << "\n";
                    return 5;
                }
                index++;
                for (j = 0; j < nargs; j++)
                {
                    if (j != nargs - 1)
                    {
                        finished = false;
                        functionStart = index;
                        while (finished == false)
                        {
                            if (index >= length)
                            {
                                cerr << "ParseError: Cannot find ','\n" << input << "\n";
                                return 6;
                            }
                            if (input[index] == ',')
                            {
                                expressionParser = new ExpressionParser();
                                expressionParserToken->parserList.push_back(expressionParser);
                                expressionParser->CreateFromString(&input[functionStart], index - functionStart);
                                finished = true;
                            }
                            index++;
                        }
                    }
                    else
                    {
                        finished = false;
                        functionStart = index;
                        parenthesisCount = 0;
                        while (finished == false)
                        {
                            if (index >= length)
                            {
                                cerr << "ParseError: Cannot find ','\n" << input << "\n";
                                return 6;
                            }
                            if (input[index] == '(') parenthesisCount++;
                            else if (input[index] == ')') parenthesisCount--;
                            if (parenthesisCount == -1)
                            {
                                expressionParser = new ExpressionParser();
                                expressionParserToken->parserList.push_back(expressionParser);
                                expressionParser->CreateFromString(&input[functionStart], index - functionStart);
                                finished = true;
                            }
                            index++;
                        }
                    }
                }
            }
            
            else if ((expectingNumber && (input[index] == '+' || input[index] == '-')
                      || strchr("01234567890.", input[index]))) // number
            {
                if (expectingNumber == false)
                {
                    cerr << "ParseError: Not expecting a number\n" << input << "\n";
                    return 4;
                }
                expectingNumber = false;
                expressionParserToken = new ExpressionParserToken();
                m_ExpressionParserTokenList.push_back(expressionParserToken);
                expressionParserToken->type = EPNumber;
                // kludge so I can use strtod
                byte = input[length];
                input[length] = 0;
                expressionParserToken->value = strtod(&input[index], &ptr);
                if (ptr == &input[index])
                {
                    cerr << "Parse Error: Could not convert number\n" << input << "\n";
                    input[length] = byte;
                    return 2;
                }
                index += (int)(ptr - &input[index]);
                input[length] = byte;
            }

            else if (strchr("+-/*!><=|&", input[index])) // operator
            {
                if (expectingNumber == true)
                {
                    cerr << "ParseError: Not expecting an operator\n" << input << "\n";
                    return 4;
                }
                expectingNumber = true;
                expressionParserToken = new ExpressionParserToken();
                m_ExpressionParserTokenList.push_back(expressionParserToken);
                expressionParserToken->type = EPOperator;
                switch(input[index])
                {
                    case '+':
                    case '-':
                    case '/':
                    case '*':
                    case '&':
                    case '|':
                    case '=':
                    case '!':
                        expressionParserToken->index = input[index];
                        index++;
                        break;

                    case '<':
                    case '>':
                        expressionParserToken->index = input[index];
                        index++;
                        if (index >= length) break;
                        if (input[index] == '=')
                        {
                            index++;
                            expressionParserToken->index += 256;
                        }
                        break;
                        
                }
            }

            else
            {
                cerr << "Parse Error: Unrecognised element\n" << input << "\n";
                return 3;
            }
        }
    }

    return 0;
}

// Evaluate expression from genome
double ExpressionParser::Evaluate(Genome *genome)
{
    return EvaluateParser(this, genome);
}

// Evaluate expression from genome
double ExpressionParser::EvaluateParser(ExpressionParser *parser, Genome *genome)
{
    double lhs, rhs;
    unsigned int i;
    
    if (parser->m_ExpressionParserTokenList.size() == 0)
    {
        cerr << "Parse Error: No tokens\n";
        return 0;
    }

    lhs = EvaluateToken(parser->m_ExpressionParserTokenList[0], genome);
    for (i = 1; i < parser->m_ExpressionParserTokenList.size() - 1; i += 2)
    {
        if (parser->m_ExpressionParserTokenList[i]->type != EPOperator)
        {
            cerr << "Parse Error: Expecting operator\n";
            return 0;
        }
        rhs = EvaluateToken(parser->m_ExpressionParserTokenList[i + 1], genome);
        lhs = EvaluateOperator(lhs, rhs, parser->m_ExpressionParserTokenList[i]->index);
    }

    return lhs;
}

// Evaluate a token
double ExpressionParser::EvaluateToken(ExpressionParserToken *token, Genome *genome)
{
    switch(token->type)
    {
        case EPNumber:
            return token->value;

        case EPParser:
            return EvaluateParser(token->parserList[0], genome);

        case EPFunction:
            return EvaluateFunction(token,  genome);

        case EPVariable:
            return EvaluateVariable(token,  genome);

        default:
            cerr << "Parse Error: Invalid token type\n";
            return 0;
    }
}

// Evaluate a variable (in this case it is always a gene)
double ExpressionParser::EvaluateVariable(ExpressionParserToken *token, Genome *genome)
{
    if (token->name != "g")
    {
        cerr << "Parse Error: Unrecognised name\n" << token->name << "\n";
        return 0;
    }
    return genome->GetGene((int)(0.5 + EvaluateParser(token->parserList[0], genome)));
}

// Evaluate a function
double ExpressionParser::EvaluateFunction(ExpressionParserToken *token, Genome *genome)
{
    switch (token->index)
    {
        case 0: // if function
            if (EvaluateParser(token->parserList[0], genome) != 0)
                return EvaluateParser(token->parserList[1], genome);
            else
                return EvaluateParser(token->parserList[2], genome);                        
        
        default:
            cerr << "Parse Error: Unrecognised function\n";
            return 0;
    }
}

// Evaluate an operator
double ExpressionParser::EvaluateOperator(double lhs, double rhs, int index)
{
    switch(index)
    {
        case '+':
            return lhs + rhs;

        case '-':
            return lhs - rhs;
            
        case '*':
            return lhs * rhs;
            
        case '/':
            return lhs / rhs;

        case '&':
            if (lhs != 0 && rhs != 0) return 1;
            else return 0;

        case '|':
            if (lhs != 0 || rhs != 0) return 1;
            else return 0;

        case '!':
            if (lhs != rhs) return 1;
            else return 0;

        case '=':
            if (lhs == rhs) return 1;
            else return 0;

        case '<':
            if (lhs < rhs) return 1;
            else return 0;

        case '>':
            if (lhs > rhs) return 1;
            else return 0;

        case ('<' + 256):
            if (lhs <= rhs) return 1;
            else return 0;

        case ('>' + 256):
            if (lhs >= rhs) return 1;
            else return 0;

        default:
            cerr << "Parse Error: Unrecognised operator\n";
            return 0;
    }
}

