#ifndef ENUM_H

#define ENUM_H

//typedef enum {false, true} bool;

typedef enum SCOPE_TYPE { GLOBAL, FUNC, LOOP, IF_BLOCK, ELSE_BLOCK, SMV_PORTS, SMV_V_MAIN ,SIMPLE_HASH} SCOPE_TYPE;

typedef enum ENTRY_TYPE {NUMBER_ENTRY, LOGICAL_ENTRY, LABEL_ENTRY ,TDS_ENTRY, T_DIRECTIVE_ENTRY, POSxSIZE ,TYPE_SET } ENTRY_TYPE;
			 // permitir lista de tipos diferentes? permitir lista de TDS's?
			 // comentado por enquanto, devemos decidir oque fazer sobre os enums do node


typedef enum smvtype {MAIN = 1, AUTOMATA = 2, PORTS = 3} smvtype;



//separar enums por: OPERAÇÃO UNÁRIA, OPERAÇÃO BINÁRIA, OPERAÇÃO ATRIBUIÇÃO, DADOS, 
					
//		DEFINIÇÃO PROGRAMA, DEFINIÇÃO TDS, DEFINIÇÃO 

									

// ou...   POR "INTERVALOS" ex:  DATA_NUM, DATA_BOOL, DATA_NULL, DATA_ITD, DATA_CTD, DATA_FTD, DATA_LABEL, DATA_V,   if(DATA_NUM  < tipo <  DATA_V) {return processData(node)}


// COLOCAR OS MAIS IMPORTANTES PRIMEIRO

typedef enum EVAL_TYPE { 

	EXPR, 

	IDVAR, NUMBER, L_BOOL, D_NULL, TIME_DIRECTIVE, STRING, DATA_V, 

	V_PROP, V_PROP_TDS,	

	TDS_DEF_COMPLETE, TD_DEF_DEPEN,

	DOMAIN_FUNCTION, TIME_LIST, TIME_COMP,

	DEF_EXTRAS_LINKED, DEF_EXTRAS_DELAYED,

	PROG, FUNC_DEFS, CMD, 

	FUNC_DEF, PROC_DEF, F_BODY, OPT_RETURN, PARAMS, PARAM, 

	CMD_IF, CMD_OTHER,
	
	CMD_TDS_ANON, TDS_ANON_OP_PASS, TDS_ANON_OP_DPASS,

	MATCH_IF, 

	OTHER_LOOP, FUNC_CALL, PROC_CALL, OTHER_ASSIGN,

	PARAMS_OP,PARAMS_CALL, PARAM_CALL,  

	AC_V, ASSIGN_TDIRECTIVE,

	CHANGE_ITD, CHANGE_CTD, CHANGE_FTD, 


} EVAL_TYPE;



#endif
