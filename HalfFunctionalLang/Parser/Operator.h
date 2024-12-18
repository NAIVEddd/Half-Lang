#pragma once
#include"Primitives.h"
#include"CharParsers.h"
#include<map>
#include<stack>

template<typename A>
void RemoveIndex(std::vector<A>& vec, int index)
{
	for (size_t i = index; i < vec.size() - 1; i++)
	{
		vec[i] = vec[i + 1];
	}
	vec.resize(vec.size() - 1);
}

template<typename Term, typename After>
struct Operator
{
	enum class Category
	{
		Infix,
		Prefix,
		Postfix
	};
	Category category;
	Term term;
	After after;
	Converter2<Term, Term, Term> converter;
};

template<typename Term>
struct InfixOperator
{
	std::string operatorString;
	int precedence;
	Converter2<Term, Term, Term> converter;
	InfixOperator() = default;
	InfixOperator(std::string op_, int precedence_, Converter2<Term, Term, Term> conv)
		: operatorString(op_), precedence(precedence_), converter(conv)
	{
	}
};

template<typename Term, typename A = ParserResult_t<Term>>
class OperatorParser
{
public:
	OperatorParser(Term t) : term(t){}
	~OperatorParser(){}
	void AddOperator(InfixOperator<A> op)
	{
		ops.insert({ op.operatorString, op });
	}
	Parser<A> GetParser()
	{
		return [=](ParserInput s) -> ParserResult<A>
			{
				std::vector<A> terms;
				std::vector<std::string> opStrings;
								
				auto punctuation = ManySatisfy1(IsPunctuation) < Spaces();

				auto termResult = term(s);
				if (!termResult)
				{
					return std::nullopt;
				}

				// save term
				terms.push_back(termResult.value().first);
				ParserInput input = termResult.value().second;

				// parse `[+ term]*`
				while (!s.empty())
				{
					// parse `+-*/`
					auto puncResult = punctuation(input);
					if (!puncResult)
					{
						break;
					}
					opStrings.push_back(puncResult.value().first);

					input = puncResult.value().second;
					termResult = term(input);
					if (!termResult)
					{
						// parse `(...term...)`
						auto begin = OneChar('(') < Spaces();
						auto beginResult = begin(input);
						if (!beginResult)
						{
							break;
						}
						input = beginResult.value().second;
						auto recLambda = GetParser();
						termResult = recLambda(input);
						if (!termResult)
						{
							break;
						}
						// save term
						terms.push_back(termResult.value().first);

						input = termResult.value().second;
						auto end = OneChar(')') < Spaces();
						auto endResult = end(input);
						if (!endResult)
						{
							break;
						}

						input = endResult.value().second;
						continue;
					}

					// save term
					terms.push_back(termResult.value().first);
					input = termResult.value().second;
				}

				if (opStrings.empty())
				{
					return std::nullopt;
				}

				// process with precedence
				ShrinkOP(terms, opStrings);

				return std::make_pair(terms[0], input);
			};
	}

	void ShrinkOP(std::vector<A>& terms, std::vector<std::string>& opStrings)
	{
		while (terms.size() > 1)
		{
			ShrinkOP(terms, opStrings, 0);
		}
	}

	void ShrinkOP(std::vector<A>& terms, std::vector<std::string>& opStrings, int start)
	{
        // error report:
		//		if opStrings is not find, print error message and interrupt the program
        for (auto& op : opStrings)
        {
            if (ops.find(op) == ops.end())
            {
                printf("Operator %s not found\n", op.c_str());
                exit(1);
            }
        }

		if (opStrings.size() == 1)
		{
			terms[0] = ops[opStrings[0]].converter(terms[0], terms[1]);
			terms.resize(terms.size() - 1);
			opStrings.resize(opStrings.size() - 1);
			return;
		}
		auto& op1 = ops[opStrings[start]];
		auto& op2 = ops[opStrings[start + 1]];

		if (op1.precedence < op2.precedence)
		{
			if ((start + 2) < opStrings.size())
			{
				ShrinkOP(terms, opStrings, start + 1);
				//ShrinkOP(terms, opStrings, start);
			}
			else
			{
				auto r = op2.converter(terms[start + 1], terms[start + 2]);
				terms[start + 1] = r;
				RemoveIndex(terms, start + 2);
				RemoveIndex(opStrings, start + 1);
			}
		}
		else // op1.precedence >= op2.precedence
		{
			auto l = op1.converter(terms[start], terms[start + 1]);
			terms[start] = l;
			RemoveIndex(terms, start+1);
			RemoveIndex(opStrings, start);
		}
	}

private:
	Term term;
	std::map<std::string, InfixOperator<A>> ops;
};