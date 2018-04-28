// This class is used for parsing simple text math expressions

#ifndef ExpressionParser_h
#define ExpressionParser_h

#include <vector>
#include <string>

class ExpressionParserToken;
class Genome;

enum ExpressionParserTokenType
{
    EPFunction = 0,
    EPOperator = 1,
    EPNumber = 2,
    EPVariable = 3,
    EPParser = 4
};

class ExpressionParser
{
public:
    ExpressionParser();
    ~ExpressionParser();

    int CreateFromString(char *input, int length);
    double Evaluate(Genome *genome);
    
protected:

    static double EvaluateParser(ExpressionParser *parser, Genome *genome);
    static double EvaluateToken(ExpressionParserToken *token, Genome *genome);
    static double EvaluateVariable(ExpressionParserToken *token, Genome *genome);
    static double EvaluateFunction(ExpressionParserToken *token, Genome *genome);
    static double EvaluateOperator(double lhs, double rhs, int index);    

    vector<ExpressionParserToken *> m_ExpressionParserTokenList;
};

struct ExpressionParserToken
{
public:
    ExpressionParserToken() {value = -1;};
    ~ExpressionParserToken()
    {
        for (unsigned int i = 0; i < parserList.size(); i++) delete parserList[i];
    }
    
    ExpressionParserTokenType type;
    double value;
    int index;
    string name;
    vector<ExpressionParser *> parserList;
};

#endif

