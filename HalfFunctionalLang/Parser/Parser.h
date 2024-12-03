#pragma once
#include<iostream>

enum class ParserState
{
	Success,
	Failure
};

template<typename Result, typename UserState>
struct ParserResult
{
	ParserState ParserState;
	Result Result;
	UserState UserState;
};
