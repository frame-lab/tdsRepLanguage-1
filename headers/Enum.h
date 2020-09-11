#ifndef ENUM_H

#define ENUM_H

//typedef enum {false, true} bool;

typedef enum SCOPE_TYPE { GLOBAL, FUNC, LOOP, IF_BLOCK, ELSE_BLOCK} SCOPE_TYPE;

typedef enum ENTRY_TYPE {NUMBER_ENTRY, LOGICAL_ENTRY, TDS_ENTRY, T_DIRECTIVE_ENTRY} ENTRY_TYPE;

typedef enum smvtype {MAIN = 1, AUTOMATA = 2, PORTS = 3} smvtype;

typedef enum NODE_TYPE { 


	PROG, FUNC_DEFS, CMDS, 

	FUNC_DEF, PROC_DEF, BODY, OPT_RETURN, PARAMS, PARAM, 

	CMD_IF, CMD_OTHER,
	
	CMD_TDS_ANON, TDS_ANON_OP_PASS, TDS_ANON_OP_DPASS,

	MATCH_IF, 

	OTHER_LOOP, FUNC_CALL, PROC_CALL, OTHER_ASSIGN,

	PARAMS_CALL, PARAM_CALL,  

	ASSIGN_V, ASSIGN_TDIRECTIVE,

	CHANGE_ITD, CHANGE_CTD, CHANGE_FTD, 

	EXPR_NEG, EXPR_SUB, EXPR_ADD, 

	MULTI_TIMES, MULTI_DIV, MULTI_MOD, 

	INEQ, INEQ_LE, INEQ_GE, INEQ_LT, INEQ_GT, INEQ_EQ, INEQ_NEQ,

	LOGICAL_EXP, LOGICAL_NOT, LOGICAL_AND, LOGICAL_OR, 

	DATA_NUM, DATA_BOOL, DATA_NULL, DATA_ITD, DATA_CTD, DATA_FTD, DATA_LABEL, DATA_V, 

	V_SIMPLE_OR_PROP, V_LIST, V_PROP, V_PROP_TDS,

	PROP_TDS_FUNC, PROP_TDS_PORT, PROP_TDS_DATA, PROP_TDS_LINKED, PROP_TDS_DELAYED,

	TDS_DEF_COMPLETE, TD_DEF_DEPEN, TDS_DEF_EXTRAS,

	DEF_DATA_FLOW, DOMAIN_TIME_LIST, DOMAIN_FUNCTION,

	TIME_LIST_COMP_LOOP, TIME_LIST, TIME_COMP,

	DEF_EXTRAS_LINKED, DEF_EXTRAS_DELAYED, 

	EXTRAS_DELAYED_LOOP, EXTRAS_DELAYED

} NODE_TYPE;



#endif
