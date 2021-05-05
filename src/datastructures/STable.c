#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../headers/STable.h"


const char* mappingEnumTable[] =  {
  "GLOBAL", "FUNC", "LOOP", "IF_BLOCK", "ELSE_BLOCK", "SMV_PORTS", "SMV_V_MAIN" ,"SIMPLE_HASH"
};


// entry
TableEntry* createEntry(char* name, Object* val, int methodParam, STable* parentScope) {
    
    TableEntry* newEntry = (TableEntry*) malloc(sizeof(TableEntry));
    
    int s = strlen(name);
    //printf("[createEntry] nome original : %s \n",name);
    //printf("[createEntry] tamanho nome: %d \n",s);


    // agora ele sempre aloca um nome novo, deve-se dar free também!
    char* newName = malloc(sizeof(char)*(s+1));
    int i;

    char* dest = newName; 
    char* src = name;
    for(i=0; i < s; i++)
    {
    	// enquanto não \0
    	if(*src)
    	{
    		*dest = *src;
    		dest++;
    		src++;
    	}

    }
    *dest = '\0';

    //printf("[createEntry] novo nome: %s \n",newName);
    
    newEntry->name = newName; 
    newEntry->val = val;   // objects sempre vão ser alocados, string devemos ter tratativa! (a não ser que essa seja alocada pelo bison?)
    newEntry->methodParam = methodParam;
    newEntry->parentScope = parentScope;
    newEntry->level = parentScope->level;
  	newEntry->order = parentScope->order;
    
    
    return newEntry;
    
    
}


void printEntry(TableEntry* e) {

	int info = e == NULL ? 0 : 1;
	//printf("teste : %d \n",info);
	if(info) 
	{
		printf("\t %s : ( ",e->name);
		printObject(e->val);	
		if(e->val->type == TYPE_SET)
		{
			//printTable((STable*)e->val->values[2]);
		}
		printf(")");
		printf("\n");
		printf("\t (context: %d, methodParam: %d, level: %d, order: %d ) \n",e->val->timeContext,e->methodParam,e->level,e->order);

	}
}



/// table

STable *createTable(SCOPE_TYPE type, STable *parent, int level, int order, int indexRef) {

	STable* newtable = (STable*) malloc(sizeof(STable));
	
/*	
	int nt = sizechildren /sizeof (STable*);
	STable** chillist;
	if(nt){
	    chillist = (STable**) malloc(nt*sizeof(STable*));   
	    // quais situações? PASSAR FILHOS COM VAR_ARGS ? (parece complicar desnecessáriamente...)(tabelas não vao ser criadas já com escopos filhos!)
	    // se não for feito assim vai literalmente não ter nenhuma utilidade passar os filhos para o construtor?
	}
*/
	newtable->nchild = 0;
	newtable->type = type;
	newtable->level = level;
	newtable->order = order;

	newtable->type = type;
	newtable->level = level;
	newtable->order = order;
	newtable->lastEntryIndex = 0;
	newtable->backup = 0;
	newtable->collision = 0;
	newtable->conditionBind = NULL;
	newtable->notEvaluated = 0;
	newtable->notWrite = 0;
	newtable->children = NULL;
	
	newtable->parent = parent;
    newtable->childOfFunction = parent && (parent->type == FUNC || parent->childOfFunction);
    newtable->indexRef = newtable->type == FUNC? indexRef : newtable->childOfFunction? parent->indexRef : -1;

/*
	if(chillist){
		newtable->children = chillist;
	}
*/




/*
	if(type == SIMPLE_HASH)
	{
		printf("[createTable] criando simple calculateHashPos \n\n");
	}
	else
	{
		printf("[createTable] criando escopo \n\n");
	}
*/
	
	newtable->tableData = (TableEntry**) malloc(MAX_TABLE*sizeof(TableEntry*));
	
	
	// garantia (tudo bem que eu NÃO VOU PRECISAR PERCORRER A TABELA DE SIMBOLOS, mas ele ta quebrando no print (por existir "qualquer coisa na tabela"))
	int i;
	for (i = 0; i < MAX_TABLE; i++)
	{
		newtable->tableData[i] = NULL;	
	}


	return newtable;

}

void printTable(STable* t){
	if(t){
		printf("%s (%d,%d)",mappingEnumTable[t->type],t->level,t->order);
		if(t->lastEntryIndex != 0 && t->tableData){
			printf("|--> Entries: \n");
			int i;
			//printf("(total) %d \n",t->lastEntryIndex);
			for(i=0;i<= t->lastEntryIndex; i++){
				if(t->tableData[i])
				{
					//printf("%d \n",i);
					printEntry(t->tableData[i]);
				}
				
			}
		}
		else{
            printf("|--> Entries: {NONE} \n");
		}
		if(t->nchild)
		{
			int i;
			for (i = 0; i < t->nchild; i++)
			{
				printf("\tchild (%d): \n",i+1);
				printf("\t\t");
				printTable(t->children[i]);
			}
		}
	}
}

void letGoEntryByName(STable* table, char* name){
    int index = calculateHashPos(name, table);
    letgoEntry(table->tableData[index]);
    table->tableData[index] = NULL;
}

void letgoEntry(TableEntry *e) {
	if(!e) {
	    return;
	}
	if(e->name)
	{
		free(e->name);
	}
	if(e->val)
	{
	    //if(e->val->type == TYPE_SET){
        //    letgoTable((STable *) e->val->values[2]);
	    //}
        letgoObject(e->val);
	}
	free(e);
}


void letgoTable(STable *t)
{
	if(!t) {
	    return;
	}
	if(t->parent){
	    t->parent->children[t->order] = NULL;
	}
	int i;
	if(t->children){
		for(i=0; i < t->nchild; i++){
            letgoTable(t->children[i]);
		}
		free(t->children);
	}
	if(t->tableData){
		//int size = t->type == SIMPLE_HASH || t->type == SMV_PORTS?  MAX_SIMPLE : MAX_TABLE;
		for(i=0; i < MAX_TABLE; i++)
		{
		    if(t->tableData[i]) {
                letgoEntry(t->tableData[i]);
			}
		}
	    free(t->tableData);
	}
    if(t->conditionBind){
        free(t->conditionBind);
    }
	free(t);
}







int calculateHashPos(char * str, STable* t) {

	int SIZE_FOR_HASH = t && t->collision ? t->collision : MAX_TABLE;
	if(t && t->collision)
	{
		printf("[calculateHashPos] collision : (%d) \n",SIZE_FOR_HASH);
	}
	return hash(str,SIZE_FOR_HASH);
}


void redistributeHashs(STable* t, TableEntry* e)
{
	t->lastEntryIndex = 0;
	int i;
	int originalSize = t->collision? t->collision :  MAX_TABLE; // salvamos o indice de colisão original ou o MAX_TABLE para fazer a redistribuição

	// atualizamos o novo indice de colisão (ou é o dobro ou é o dobro do original)
	t->collision = t->collision?  t->collision*2 : MAX_TABLE*2;	
	TableEntry** newTableData = (TableEntry**) malloc(t->collision*sizeof(TableEntry*));
	
	//"limpar"
	for (i = 0; i < t->collision; i++)
	{
		newTableData[i] = NULL;	
	}	

	printf("[redistributeHashs] copiando valores anteriores \n");

	for (i = 0; i < originalSize; i++)
	{
		if(t->tableData[i])
		{
			int index = calculateHashPos(t->tableData[i]->name, t);
			printf("[redistributeHashs] new index (%s) %d \n",t->tableData[i]->name,index);
			newTableData[index] = t->tableData[i];
		}
	}

	//libera a região de memória antiga
	free(t->tableData);

	//registra a nova
	t->tableData = newTableData;

	// tenta inserir o membro novo novamente
	insert(t,e);
    	
}

void insert(STable* t, TableEntry* e) {
    
    int index = calculateHashPos(e->name, t);
    //printf("[insert] HASH CALCULADO para (%s) É: %d \n",e->name,index);
    
    if(lookup(t,e->name))
    {
    	printf("[insert] COLLISION! (%s) É: %d \n",e->name,index);
    	exit(-1);
    	//redistributeHashs(t,e);
    }
    else
    {
    	t->tableData[index] = e;
    	t->lastEntryIndex = index > t->lastEntryIndex ? index : t->lastEntryIndex;
    }
   	
    
}


TableEntry* lookup(STable* t, char* name) {
    
    int index = calculateHashPos(name, t);
    TableEntry* e = t->tableData[index];
    if(e)
    {
    	return e;
    }
    // não achou procura na hierarquia de escopos acima
    //printf("[lookup] WARNING %s not in scope : ",name);
    //printf("%s (%d,%d) \n",mappingEnumTable[t->type],t->level,t->order);
    STable* parent = t->parent;
   	while(!e && parent)
    {
    	//printf("[lookup] parent: %s (%d,%d)",mappingEnumTable[parent->type],parent->level,parent->order);
    	e = parent->tableData[index];
    	//printTable(parent);
    	parent = parent->parent;
    }
   	return e;
}


void addNumericalIntervalSmv(char* name, int pos, int tam, int pointIni, int pointEnd, int min , int max, int newValue, STable* current){

    //printf("[addNumericalIntervalSmv] add var-name: %s to %s \n",name,mappingEnumTable[current->type]);

    max = newValue > max && newValue > min ? newValue : max;
    max = newValue < max && newValue < min ? newValue : min;

    void* po[] = {&pos, &tam, &pointIni, &pointEnd, &min, &max};
    addValue(name, po, WRITE_SMV_INFO, 4, 0, current, 0);
}

/*
void addWriteInfo(char* name, void** any, int any_type, int object_size, STable* current)
{
    printf("[addWriteInfo] add var-name: %s to %s \n",name,mappingEnumTable[current->type]);
    STable* hashset = createTable(SIMPLE_HASH,NULL,0,0);

    void* po[] = {any[0], any[1], hashset};

    printf("[addTypeSetSmv] (index: %d, size: %d) \n",*(int*)po[0],*(int*)po[1]);

    addValue(name,po,any_type,object_size+1,0,current);
}
*/

void addSmvInfoDeclaration(char *name, void **any, int any_type, int object_size, STable *current, void* type_smv_info_dc){
    addValue(name,any,any_type,object_size,0,current,-1);
    Object* VAR_DECLARATION_SMV_INFO = lookup(current,name)->val;
    void* copy_type_smv_info_dc = NULL;
    if(any_type == WRITE_SMV_INFO){
       copy_type_smv_info_dc = copyTypeMinMax(type_smv_info_dc);
    }
    else{
        copy_type_smv_info_dc = copyTypeSet(type_smv_info_dc);
    }
    VAR_DECLARATION_SMV_INFO->type_smv_info = copy_type_smv_info_dc;
}

// refatorar? os dois métodos, usar só um que recebe "qualquer coisa" e encapsula em um objeto
void addValue(char *name, void **any, int any_type, int object_size, int methodParam, STable *current, int timeContext)
{

	// note que po é um ponteiro para objetos que o novo objeto irá encapsular, como criar ?
	//void* pa[] = {&vali}; (pro :possibilita manipular arrays) (cons: tenho que tratar tudo como vetor até quando é um unico valor)


	Object* o = createObject(any_type, object_size, any, timeContext, name, NULL);
	addValueCurrentScope(name,o,methodParam,current);
}


void updateValue(char *name, void **any, int any_type, int object_size, int oIndex, int oProp, STable *current, int contextChange)
{
	TableEntry* e = lookup(current,name);
	if(e){
        //printf("[updateValue]  newValue  %d \n",*(int*)any[0]);
        updateObject(e->val, any, any_type, object_size, oIndex, oProp, contextChange);
	}
	else{
	   if(oIndex > 0){
           fprintf(stderr, "ASSIGN ERROR: %s is not defined for reference %s[%d] \n", name,name,oIndex);
           exit(-1);
	   }
	   addValue(name, any, any_type, object_size, 0, current, 0);
	}

}

void addReferenceCurrentScope(char* name, Object* DATA_STRUCT, int methodParam, STable* current){
    if(lookup(current,name)){
        fprintf(stderr,"%s already declared!\n",name);
        exit(-1);
    }
    addValueCurrentScope(name,DATA_STRUCT,methodParam,current);
}


void addValueCurrentScope(char* name, Object* val, int methodParam,STable* current) {

	//nome da variavel,   val vai ser literalmente o valor dela (problema, e quanto for uma lista?)
	TableEntry* entry = createEntry(name,val,methodParam,current);
	//printf("[addValueCurrentScope] DEBUG: \n\n");
	//printEntry(entry);
	insert(current,entry);
}



/*
	Depois acho que vale a pena melhorar isso aqui e não usar realloc, 
	justificativa: nem nos casos "mais extremos" teremos muitos filhos existindo "ao mesmo tempo" em um escopo, isto é,
	desde que eu sempre destrua os escopos filhos depois que eles são usados o seguinte não vai precisar se preocupar 

*/


//void ou retorna o filho??? Retornar parece melhor, provavelmente ao adicionar um subscope eu vou querer operar sobre ele imediatamente
STable* addSubScope(STable* parent, SCOPE_TYPE type) {
	
	// LEMBRE-SE nchild usa a próxima posição para o escopo filho (inicia em 0, primeira pos)

	STable* child = createTable(type, parent, parent->level + 1, parent->nchild, -1);
	
	//printf("[addSubScope] alocando filho: %d \n",parent->nchild);

	if(!parent->nchild) {
		parent->children = (STable**) malloc((parent->nchild+1)*sizeof(STable*));
	}
	else
	{
		if(!parent->backup)
		{
			//printf("[addSubScope] realocando filhos: %d \n",parent->nchild);
			STable** newbuffer = realloc(parent->children, (parent->nchild+1)*sizeof(STable*));
		
			if(newbuffer == NULL)
			{
				//back up em caso de falha de alocação de escopo
				int i;
				printf("[ALERT: addSubScope]  backup children \n");
				newbuffer = (STable**) malloc(MAX_CHILD*sizeof(STable*));
				for (i = 0; i < parent->nchild; i++)
				{
					newbuffer[i] = parent->children[i];
				}
				free(parent->children);
				// copia com espaço máximo (para de realocar)
				parent->children = newbuffer;
				parent->backup = 1;
			}
			else
			{
				parent->children = newbuffer;
				// realloc já da free na memória antiga !
			}
		}
		// senão não necessita alocar indefinidamente
	}
	parent->children[parent->nchild] = child; // adiciona novo filho
	// dita a próxima posição
	parent->nchild++;

	return child;

}

// retorna o pai para a gente voltar a "ter como operar" nele (talvez não necessite porque a função de pós processamento é recursiva)
// pode necessitar melhorias depois
STable * letgoSubScope(STable* current)
{
	STable* parent = current->parent;
    letgoTable(current);
	parent->nchild --;

	return parent;

}

