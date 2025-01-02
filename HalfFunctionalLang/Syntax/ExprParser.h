#pragma once
#include"../Parser/Primitives.h"
#include"../Parser/CharParsers.h"
#include"../Parser/Operator.h"
#include"Base.h"

ParserResult<Half_Expr> pmanyexpr(ParserInput s);
ParserResult<Half_Expr> pexpr(ParserInput s);
ParserResult<Half_Expr> pprogram(ParserInput s);

template<typename A>
Half_Expr ConvertToExpr(A a)
{
	return std::move(Half_Expr(a));
}

template<typename T, typename A = ParserResult_t<T>>
Parser<Half_Expr> PipeExpr(T t)
{
	return [=](ParserInput s) -> ParserResult<Half_Expr>
		{
			auto p = Pipe<T, Half_Expr>(t, (Converter<A, Half_Expr>) ConvertToExpr<A>);
			return p(s);
		};
}

/// <summary>
/// Parse Half_Value
/// </summary>
/// <param name="s"></param>
/// <returns>Half_Value</returns>

ParserResult<Half_Value> pchar(ParserInput s);

ParserResult<Half_Value> pstring(ParserInput s);

ParserResult<Half_Value> pint(ParserInput s);

ParserResult<Half_Value> pfloat(ParserInput s);

ParserResult<Half_Value> pvalue(ParserInput s);

ParserResult<std::string> pvariablename(ParserInput s);
ParserResult<std::string> ptypename(ParserInput s);

ParserResult<std::string> pfieldname(ParserInput s);

ParserResult<std::string> psubscriptstring(ParserInput s);

ParserResult<Half_Var> psimplevar(ParserInput s);

ParserResult<Half_Var> pvar(ParserInput s);

ParserResult<Half_Funcall> pfuncall(ParserInput s);

ParserResult<Half_Op> pop(ParserInput s);

ParserResult<Half_ArrayInit> parrayinit(ParserInput s);

ParserResult<Half_ArrayNew> parraynew(ParserInput s);

ParserResult<Half_StructInit> pstructinitbody(ParserInput s);

ParserResult<Half_Assign> passign(ParserInput s);

ParserResult<Half_If> pif(ParserInput s);

ParserResult<Half_FuncDecl> pfuncdecl(ParserInput s);

ParserResult<Half_TypeDecl::Nil> pniltype(ParserInput s);
ParserResult<Half_TypeDecl::TupleType> ptupletype(ParserInput s);
ParserResult<Half_TypeDecl::Additional> padditionaltype(ParserInput s);
ParserResult<Half_TypeDecl::BasicType> pbasictype(ParserInput s);
ParserResult<Half_TypeDecl::Ptr> ppointertype(ParserInput s);
ParserResult<Half_TypeDecl::IncompleteArrayType> pincompletetype(ParserInput s);
ParserResult<Half_TypeDecl::ArrayType> parraytype(ParserInput s);
ParserResult<Half_TypeDecl::StructType> pstructbody(ParserInput s);
ParserResult<Half_TypeDecl::FuncType> pfunctype(ParserInput s);

ParserResult<Half_TypeDecl> ptypeuse(ParserInput s);

ParserResult<Half_TypeDecl> ptypedecl(ParserInput s);

ParserResult<Def_Type> pdeftype(ParserInput s);

ParserResult<Half_Let> plet(ParserInput s);

ParserResult<Half_For> pfor(ParserInput s);

ParserResult<Half_While> pwhile(ParserInput s);
