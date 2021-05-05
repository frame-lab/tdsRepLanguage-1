//
// Created by mateus on 20/04/2021.
//

#ifndef TDSREPLANGUAGE_LANGCORE_H
#define TDSREPLANGUAGE_LANGCORE_H

#include "Visitor.h"


//#define DEFAULT_MEMOI 300


//Object** MEMOI;


/*
 * FUNÇÕES AUXILIARES
 */

STable* selectSMV_SCOPE(STable* scope, EnvController* controllerSmv);

HeaderSmv * selectSMV_INFO(STable* scope, Object* functionPointer, EnvController* controllerSmv);
// avaliar a importancia dos métodos acima, e se possível movimentar eles para o HeaderSmv.c ou para um Novo Controller.h

/**
 * Resolve a dependencia de todas as TDS's linkadas a TDS corrente durante a avaliação lazy
 * @param currentTDS a TDS corrente
 * @param controllerSmv o controller usado para atualizar o type-set das TDS's dependentes.
 * @param scope a  tabela de simbolos do escopo corrente
 * @param I_TIME para validar o instante ao criar init e next
 * @param C_TIME para indexar o data-time correto
 */
void resolveDependencies(TDS* currentTDS, STable* scope,  EnvController* controllerSmv, int I_TIME, int C_TIME);

/**
 * Resolve a avaliação lazy das TDS's pendentes após uma mudança de contexto temporal. E salva seus dados para Memoization.
 * @param currentScope o escopo atual
 * @param controllerSmv o controlador de ambiente
 * @param C_TIME o contexto temporal atual
 */
void resolveTdsLazyEvaluation(STable *currentScope, EnvController *controllerSmv, int C_TIME);

// CASO FORA DE FLUXO 1: e se ele "pular", ex: commitar c_time = 2 (quando era 0 antes), ele pulou o 1! A gente deve ver a "diferença"
// CHAMAR ESSA FUNÇÃO DIFERENÇA VEZES.
// CASO FORA DE FLUXO 2: deve chamar isso também ao FIM DO PROGRAMA (para cada CHAMADA RESTANTE, ex: C_TIME terminou em 4, deve chamar até F_TIME (5, ... , F_TIME).
// DIFERENÇA VEZES DE NOVO!
void commitCurrentTime(STable* currentScope, EnvController* controllerSmv, int changedTo);

/**
 * Real as operaçoes basicas envolvidas em quaisquer declaraçoes de TDS's. Dentre elas, alocaçao da estrutura,
 * consulta das diretivas temporais, sintetização de binds e, se necessário, resolução de dependências.
 * @param pathForDepen o path para se resolver uma dependência, isto é o dos nós mais abaixo da AST
 * @param portName o nome da porta
 * @param type o tipo da TDS para enumeraçao
 * @param tdsSpec o objeto que representa a especificaçao da TDS
 * @param delayed o parametro delayed de uma TDS
 * @param scope o scopo corrente
 * @param pathForCond o caminho para recuperar a condição a ser avaliada para essa TDS
 * @param controller o controlador de ambiente
 * @return um objeto sintetizado com a TDS criada.
 * @SideEffects: Aloca uma TDS, e posições no vetor de dependências de uma TDS se for necessário
 */
Object *computeTDSBasicOperations(Node *pathForDepen, char *portName, TDS_TYPE type, Object *tdsSpec, int delayed, STable *scope, Node *pathForCond, EnvController *controller);


/**
 *
 * @param n o nó passado para resolução de dependências
 * @param portName o nome da porta da tds
 * @param scope o escopo corrente
 * @param newTDS a nova TDS
 * @param controller o controlador de ambiente
 * @param I_TIME a diretiva de tempo inicial
 * @param C_TIME a diretiva de tempo corrente
 * @return a TDS com as dependencias computadas
 * @SideEffects: Aloca e posições no vetor de dependências de uma TDS se for necessário
 */
void computeTDSDependentOperations(Node*n, char* portName, STable* scope, TDS* newTDS, EnvController* controller, int I_TIME, int C_TIME);


/**
 * Realiza a negação de uma expressão (object)
 * @param o a expressão object
 * @return um objeto que encapsula true ou false
 * @Validation:  Caso o object não seja booleano dispara erro.
 */
Object * notObjectOperation(Object* o);

/**
 * Realiza a comparação entre dois objetos
 * @param o1 o primeiro operando
 * @param o2 o segundo operando
 * @param op diferencia se é diferente de e igual a
 * @return um objeto que encapsula true ou false
 * @Validation: Caso os objetos sejam de tipos diferentes dispara erro.
 */
int comparator(Object* o1, Object* o2, char* op);

/**
 * Implementa implicação lógica
 * @param a primeiro predicado
 * @param b segundo predicado
 * @return um resultado 0 ou 1
 */
_Bool IMP(_Bool a, _Bool b);

/**
 * Implementa biimplicação lógica
 * @param a primeiro predicado
 * @param b segundo predicado
 * @return um resultado 0 ou 1
 */
_Bool BIMP(_Bool a, _Bool b);


/**
 * Método auxiliar para tratar atualização de valores de variáveis, incluindo de NULL e listas
 * @param varName o nome da variável
 * @param var o objeto referência da variável
 * @param expr o objeto referência da expressão (que será atirbuida a var)
 * @param scope o escopo atual
 * @param index o indice o qual a atribuição é feita caso seja uma lista
 * @param C_TIME a diretiva C_TIME
 * @SideEffects: Atualiza o valor de uma entrada na tabela de simbolos
 */
void updateVariable(char* varName, Object *var, Object *expr, STable* scope, int index, int C_TIME);

/**
 * Considerando as limitações de enums simbolicos do nuXmv,
 * enquanto o modelo gerado utilizar esse tipo de dado essa validação deve ser utilizada para
 * qualquer operação diferente de != e ==. Abortando a interpretação.
 * @param o1 objeto sintetizado em uma operação
 * @param o2 outro objeto sintetizado em uma operação
 *
 */
void validateOpsTdsValue(Object* o1, Object* o2);

#endif //TDSREPLANGUAGE_LANGCORE_H
