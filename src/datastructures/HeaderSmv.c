#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../headers/HeaderSmv.h"
#include "../../headers/Enum.h"
#include "../../headers/textManager.h"

HeaderSmv* createHeader(int type, long moduleP, long varP, long assignP, long transP, long invarPointer, long offset){

  HeaderSmv* header = (HeaderSmv*) malloc(sizeof(HeaderSmv));
  header->type = type;
  header->modulePointer = moduleP;
  header->varPointer = varP;
  header->assignPointer = assignP;
  header->transPointer = transP;
  header->invarPointer = invarPointer;
  header->CURR_OFFSET = offset; // variavel auxiliar para ajudar a sobrescrever arquivos?

  return header; 


}

void printHeader(HeaderSmv* h){

  if(h) {
		
		printf("%d: MODULE() - %ld \n",h->type,h->modulePointer);

		printf(" VAR - %ld \n",h->varPointer);
	
		printf(" ASSIGN - %ld \n",h->assignPointer);
	
		printf(" TRANS - %ld \n",h->transPointer);

		printf(" INVARVAR - %ld \n",h->invarPointer);
	
		printf(" OFFSET - %ld \n",h->CURR_OFFSET);
 	
 }
}

void letgoHeader(HeaderSmv* h){

	if(!h){
		return;
	}
	free(h);


}


HeaderSmv** initHeadersStruct(int size){
  
  HeaderSmv** headers = ( HeaderSmv**) malloc(sizeof(HeaderSmv*)*size);
  return headers;
}

void letGoHeadersStruct(HeaderSmv** hs, int size){
  
  int i;
  for(i = 0; i< size; i++){
	letgoHeader(hs[i]);
	hs[i] = NULL;
  }
  free(hs);
}


char* clearOldPortsRefs(char* oldConstraint) {
    
    char* nova = (char*) malloc(sizeof(oldConstraint));
    char* anterior = NULL;
    char* preProx;
    long offsetoriginal = strlen(oldConstraint);
//    printf("NANI!? : %s \n\n\n",nova);
    memset(nova,0,strlen(nova));
//    printf("NARUHOTO!? : %s \n\n\n",nova);
    long offsetfinal = offsetoriginal;
    long offsetEntreDeli = 0L;
    
    
    
    while(*oldConstraint){
        
        preProx = customCat(nova,oldConstraint,'[',0);
        if(anterior != NULL){
            oldConstraint += ((preProx - anterior)+2);
        }
        else{
            oldConstraint += (strlen(nova)+1);
        }
        
        //printf("RODADA I \n\n");
        //printf("o(i): %ld \n",offsetfinal);
        //printf("TESTE chegada: %s \n",preProx);
        //printf("TESTE: %s \n",nova);
        //printf("TESTE antiga: %s \n\n",oldConstraint);
        
        // terminou antes da segunda rodada (evita percorrer desnecessáriamente regiões de memória)
        if(!*oldConstraint) {
            //printf("xablau \n");
            break;
        }
        
        preProx = customCat(preProx,oldConstraint,']',1);
        
        
        oldConstraint += (preProx[0]+1);
        if(offsetEntreDeli == 0){
            offsetEntreDeli = preProx[0];
        }
        
        //printf("skiped: %d \n\n",preProx[0]+1);
        //printf("skiped: %c \n\n",preProx[0]);
        
        if(preProx[0] == offsetEntreDeli){
            offsetfinal -= (preProx[0]+2);
        }
       
        //printf("o(ii): %lu \n",offsetfinal);
        preProx[0] = '\0';
        anterior = preProx; 
        
        //printf("RODADA II \n\n");
        //printf("o(ii): %ld \n",offsetfinal);
        //printf("TESTE chegada: %s \n",preProx);
        //printf("TESTE: %s \n",nova);
        //printf("TESTE antiga: %s \n\n",oldConstraint);
        
    }
    nova[offsetfinal] = '\0';
    //printf("o: %ld \n",offsetfinal);
    return nova;
    
    
    
}


void preProcessSmv(FILE* smvP, HeaderSmv** ds) {
	
	/*Strings que são usadas para a busca no arquivo*/
	char* varString = "VAR";
	char* fVarString = "FROZENVAR";	
	char* assignString = "ASSIGN";
	char* transString = "TRANS";

	char* portsModuleString = "MODULE portsModule";
	char* automataString = "MODULE finalAutomata(time)";
	char* beginAutomataString = "MODULE ";	
	char* confirmAutomataString = "cs: {"; // se não encontrar isso após "começar a ler um automato" quer dizer que era um módulo qualquer

	/*Variáveis de controle de cursor e buffer
		-> propriedades do buffer e buffer auxiliar
		-> cursor da leitura anterior (ftell antes da leitura atual)
		-> cursor localização de cada ponto de interesse no arquivo
	*/
	
	char* buffer;
	char* buffer2;
    	size_t bufsize = 300;
	
	long prevFcursor = 0L;

	
	long moduleCursor = 0L;
  	long varCursor = 0L;
	long invarCursor = 0L;
	long assignCursor = 0L; 
	long transCursor = 0L;


	/*Variáveis de controle de "onde no arquivo estamos"
		-> estamos lendo o main
		-> estamos lendo um automato
		-> quantidade de módulos de automato
		-> na próxima vez será lida a relação de transição do automato
		-> estamos lendo a relação de transição do automato
		-> estamos lendo portsModule
		-> estamos verificando se o modulo é "importante"
		-> sabemos sobre o modulo
		-> estamos lendo algo que não importa 
		

	*/
	int readMain = 1;
	int readAutomata = 0;
	int automataCounter = 0;
	int transAutomata = 0;
	int readTrans = 0; // o !readTrans mais abaixo é usado para evitar comparações desncessárias com a string gigante de transições 
	int readPortsModule = 0;
	int evalLockText = 0;
	int evalResult = 0;
	int lock = -2;
	buffer = (char *) malloc(bufsize * sizeof(char));
   	while ((fgets(buffer,bufsize,smvP))) {
		
		if(transAutomata){
			readTrans = 1;
			transAutomata = 0;
		}

		// só feita para automatos no "inicio" da rodada
		if(readTrans) {
			// operação complexa de renomear referencias ao ports module
			if(buffer[0] == '\n' ){
				printf("OOOOH YESS \n\n");
				//printf("%s \n\n",buffer); 
				readTrans = 0; 
			}
			// atualmente só automato pode ter INVAR e após iniciar a leitura de transições
			// e que representa o fim da leitura de transições
			else if(strstr(buffer,"INVAR")){
				invarCursor = prevFcursor;
				printf("OOOOH YESS \n\n");				
				readTrans = 0;			
			}
			else {
				// COMENTADO ENQUANTO BUG NÃO É CORRIGIDO				
				//printf("OH NÕ \n\n");
				//printf("ANTES %s \n \n",buffer);				
				//buffer = clearOldPortsRefs(buffer);
				//printf("DEPOIS %s \n \n",buffer);
				//fseek(smvP,prevFcursor,SEEK_SET);
				//fputs(buffer2,smvP);
				//memset(buffer2, 0,strlen(buffer2));
				//free(buffer2);
				//readTrans = 0;
				//lock++;	
			}		
		}

		// salvar cursor de VAR
		if(!readTrans && strstr(buffer,varString)){
			varCursor = prevFcursor;
			// se for um module diferente do main(provavelmente automato) tem que avaliar
			if(readAutomata) {
				evalLockText = 1;
				//printf("INICIANDO AVALIAÇÃO DE AUTOMATO (%s) \n\n",buffer);
			} 			
		}

		// salvar cursor de FROZENVAR e trocar para VAR
		if(readPortsModule && strstr(buffer,fVarString)){
			varCursor = prevFcursor;
			fseek(smvP,varCursor,SEEK_SET); // trocar para VAR
			strcpy(buffer,varString);	
			fputs(buffer,smvP);
		}

		if(evalLockText){
			//printf("AVALIAÇÃO (%s) \n\n",buffer);			
			if(strstr(buffer,confirmAutomataString)){
				evalResult = 1;
			}
			if(evalResult){
				evalLockText = 0;			
			}
		}

		// salvar cursor de ASSIGN (e termina a criação de headers no caso de portsmodule)
		if(!readTrans && strstr(buffer,assignString)){
			assignCursor = prevFcursor;

			if(readPortsModule){
			  //clearPortsModule = 1;	
			  ds[PORTS-1]=createHeader(PORTS,moduleCursor,varCursor,assignCursor,0, 0, 0);	
			  //printf("CRIANDO PORTS HEADER INDICE %d \n\n",PORTS-1); 		
			}				
		}


		// coisas só feitas para o modulo do automato:
		if(readAutomata){
		
			//verifica se vamos ler as transições na proxima rodada do loop (topo)
			//termina de avaliar um modulo SE É AUTOMATO OU NÃO
			if(!readTrans && strstr(buffer,transString)) {
				transCursor = prevFcursor;
				if(!evalResult){
					readAutomata = 0; // o módulo na verdade não é um automato
					//printf("MODULO NÃO AUTOMATO DETECTADO! \n\n");
				}
				else{
					//printf("MODULO  AUTOMATO DETECTADO! \n\n");					
					automataCounter++;
					transAutomata = 1;					
				}					
			}		
		}
				

		// termina a leitura de MAIN ou termina a leitura de outro MÓDULO 
		if(prevFcursor != 0L && strstr(buffer,beginAutomataString)){			
			if(readMain) {
				moduleCursor = prevFcursor; // salva o inicio do módulo !			
				ds[MAIN-1] = createHeader(MAIN, 0, varCursor, assignCursor, 0, 0, 0);	 // salva o main lido anteriormente
				readMain = 0;	
				readAutomata = 1;				
				//printf("%s \n",buffer);
			}

			if(readAutomata) {
				readTrans = 0;	       
				if(automataCounter == 1){
				  	ds[AUTOMATA-1]=createHeader(AUTOMATA,moduleCursor,varCursor,assignCursor,transCursor, invarCursor, 0); 
				 	//printf("CRIANDO AUTOMATO HEADER INDICE(i) %d \n\n",AUTOMATA-1); 
				}
				else if (automataCounter != 0){
				 	//printf("CRIANDO AUTOMATO HEADER INDICE(ii) %d \n\n",AUTOMATA+(automataCounter-2));
				 	ds[AUTOMATA+(automataCounter-2)]=
						createHeader(AUTOMATA,moduleCursor,varCursor,assignCursor,transCursor, invarCursor, 0);
				}
				moduleCursor = prevFcursor;				
			}
			
			// chegamos em ports module...
			if(strstr(buffer,portsModuleString)) {
				//printf("HERE WE ARE! %ld \n\n",prevFcursor);
				readAutomata = 0;
				readTrans = 0;	
				readPortsModule = 1;				
				// escrever time em ports module!			
				//printf("LINHA : %s \n",buffer);
				//buffer = addParamModule(buffer,"time");
				//printf("NOVO : %s \n",buffer);
				//fseek(smvP,prevFcursor,SEEK_SET); // voltar para a leitura anterior!
				//fputs(buffer,smvP);		  // sobrescrever ports module, cursor volta a ser o atual
			}											
		} 
		prevFcursor = ftell(smvP);
	}
	free(buffer);	  
}




char* addParamModule(char* original, char* param) {
 
	char *newString;
	int breakline=0;
	if(original[strlen(original)-1] == '\n'){
	  breakline = 1;
	}

	if(original[strlen(original)-1] == ')' || original[strlen(original)-2] == ')'){
		newString = (char*) malloc(sizeof(char)*(strlen(original)+2+strlen(param)+1)); // , e  \0
		newString = customCat(newString,original,')',0);
		newString = customCat(newString,", ",0,0);
		newString = customCat(newString,param,0,0);
		newString = customCat(newString,")",0,0);		
	}
	else {
		//printf("%ld %ld \n",strlen(original),strlen(param));	   	
		newString = (char*) malloc(sizeof(char)*(strlen(original)+1+strlen(param)+1+1)); // ( , ) e  \0	   	
		newString = customCat(newString,original,'\n',0);
		newString = customCat(newString,"(",0,0);
		newString = customCat(newString,param,0,0);
		newString = customCat(newString,")",0,0); 
	}
	if(breakline){
	  newString = customCat(newString,"\n",0,0);
	}
	newString = newString - ((strlen(original)+strlen(param)+2)-1);	
	return newString;
}





