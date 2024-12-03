#pragma once
#include"IR.h"
#include"MakeIR.h"
#include"Symbol.h"
#include<memory>

Half_Ir_Exp Trans_Expr(Half_Expr& expr);
Half_Ir_Exp Trans_Expr(std::shared_ptr<Table>& table, Half_Expr& expr);