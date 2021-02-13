#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../headers/HeaderSmv.h"



HeaderSmv* createHeader(int type, char* moduleName, int varP, int assignP, int transP) {

  HeaderSmv* header = (HeaderSmv*) malloc(sizeof(HeaderSmv));

  if(type == PORTS){
      char* refOldPt;
      // só adiciona parâmetro se for portsModule
      refOldPt = moduleName;
      char* newDeclaration = addParams(refOldPt,"time","(",")");
      header->moduleName = newDeclaration;
  }
  else{
      if(type == AUTOMATA){
          header->moduleName = overwriteParam(moduleName,"ports");
      }
      else{
          char* name = malloc((strlen(moduleName)+1) * sizeof(char));
          strcpy(name, moduleName);
          header->moduleName = name;
      }
  }
  
  header->type = type;
  header->VAR_POINTER = varP;
  header->ASSIGN_POINTER = assignP;  
  header->TRANS_POINTER = transP;

  header->varBuffer = malloc(sizeof(char*)*300);
  header->assignBuffer = malloc(sizeof(char*)*300);
  if(type == AUTOMATA){
  	//printf("[createHeader] DEBUG: alocando transicoes tipo (%d) \n",type);
  	header->transBuffer = malloc(sizeof(char*)*300); // tamanho default
  }
  else{
  	header->transBuffer = NULL;
  }

//  if(type == MAIN){
//      header->VAR_RENAME_POINTER = 2;
//  }
//  else{
      header->VAR_RENAME_POINTER = -1; // ref a ports Module ficará em main
//  }

  return header; 
}

void letgoHeader(HeaderSmv* h){

	if(!h){
		return;
	}
  	//printf("[letgoHeader] DEBUG: liberando %s \n",h->moduleName);
	free(h->moduleName);
	int i;
 	
 	for(i = 0; i< h->VAR_POINTER; i++){
		if(h->varBuffer[i]){
			free(h->varBuffer[i]); 

		}
	}
	// note que poderiamos nos livrar de strings dinamicas nesse caso (reformular métodos CAT e refs para static string?)
	// not really... STRINGS SÃO PERDIDAS A NÃO SER QUE SEJAM LITERAIS ou alocadas dentro de função!
  	// ou alocar todo mundo ou tornar todas literal (provavelmente alocar todo mundo)	

  	//rintf("[letgoHeader] DEBUG: liberando buffer VAR! \n");
  	free(h->varBuffer);
	
 	for(i = 0; i< h->ASSIGN_POINTER; i++) {
		if(h->assignBuffer[i]){
			free(h->assignBuffer[i]); 
		}					 
  	}
  	//printf("[letgoHeader] DEBUG: liberando buffer ASSIGN! \n");
  	free(h->assignBuffer);		

  	if(h->transBuffer) {
		for(i = 0; i< h->TRANS_POINTER; i++) {
			//printf("???\n");
			if(h->transBuffer[i]){
				free(h->transBuffer[i]); 
			}					 
  		}
  		//printf("[letgoHeader] DEBUG: liberando buffer TRANS! \n");
  		free(h->transBuffer);
  	}
  		

  	free(h);  				
}

void letGoRelatedHeaders(HeaderSmv** RHeader, int size){
    int i;
    if(RHeader){
        for(i = 0; i< size; i++){
            letgoHeader(RHeader[i]);
            RHeader[i] = NULL;
        }
    }
}

void letGoHeadersStruct(HeaderController *H) {

    letgoHeader(H->MAIN);
    letGoRelatedHeaders(H->AUTOMATA_RELATED, H->H_AUTOMATA_CURRENT_SIZE);
    letGoRelatedHeaders(H->PORTS_RELATED, H->H_PORTS_CURRENT_SIZE);
    letGoRelatedHeaders(H->FUNCTIONS, H->H_FUNCTION_CURRENT_SIZE);

    letgoTable(H->mainInfo);
    // assim como temos n headers de função temos n tabelas de simbolos auxiliares que pode ou não ter sido liberada
    // antecipidamente (quando a função já terminou de ser escrita como smv)
    if(H->functionsInfo){
        int i;
        for (i=0; i < H->H_FUNCTION_CURRENT_SIZE; i++){
            if(H->functionsInfo[i]){
                letgoTable(H->functionsInfo[i]);
            }
        }
    }
    //letgoTable(H->portsInfo);
    letgoTable(H->originalPorts);
    free(H);
}

HeaderController *createController() {

    int DEFAULT_HEADERS_SIZE = 100;

    HeaderController* Hcontrol = malloc(sizeof(HeaderController));
    Hcontrol->H_AUTOMATA_CURRENT_SIZE = 0;
    Hcontrol->H_PORTS_CURRENT_SIZE = 0;
    Hcontrol->H_FUNCTION_CURRENT_SIZE = 0;

    Hcontrol->originalPorts = createTable(SMV_PORTS, NULL, 0, 0, -1);
    Hcontrol->mainInfo = createTable(SMV_V_MAIN, NULL, 0, 0, -1);
    Hcontrol->functionsInfo = malloc(sizeof(STable*)*DEFAULT_HEADERS_SIZE);
    int i;
    for (i = 0; i < DEFAULT_HEADERS_SIZE; i++) {
        Hcontrol->functionsInfo[i] = NULL;
    }

    //Hcontrol->portsInfo =  createTable(SMV_PORTS,NULL,0,0);

    Hcontrol->MAIN = NULL;
    Hcontrol->AUTOMATA_RELATED = malloc(sizeof(HeaderSmv*)*DEFAULT_HEADERS_SIZE);
    Hcontrol->PORTS_RELATED = malloc(sizeof(HeaderSmv*)*DEFAULT_HEADERS_SIZE);
    Hcontrol->FUNCTIONS = malloc(sizeof(HeaderSmv*)*DEFAULT_HEADERS_SIZE);

    Hcontrol->expectedPorts = 0;
    Hcontrol->declaredPorts= 0;
    return Hcontrol;
}


void letGoHeaderControl(HeaderController* Hcontrol) {
    letGoHeadersStruct(Hcontrol);
}

void printAllHeaders(HeaderController* Hcontrol){
    int i;
    printHeader(Hcontrol->MAIN);

    for (i = 0; i < Hcontrol->H_FUNCTION_CURRENT_SIZE; i++){
        printHeader(Hcontrol->FUNCTIONS[i]);
    }
    for (i = 0; i < Hcontrol->H_AUTOMATA_CURRENT_SIZE; i++){
        printHeader(Hcontrol->AUTOMATA_RELATED[i]);
    }
    for (i = 0; i < Hcontrol->H_PORTS_CURRENT_SIZE; i++){
        printHeader(Hcontrol->PORTS_RELATED[i]);
    }
}

void printHeaderBuffer(HeaderSmv* h, int part, char* partString) {
	int i;
	int offsetBuffer;
	if(part == VAR){
		offsetBuffer = h->VAR_POINTER;
	}
	if(part == ASSIGN){
		offsetBuffer = h->ASSIGN_POINTER;
	}
	if(part == TRANS){
		offsetBuffer = h->TRANS_POINTER;
	}

 	printf("------------BUFFER %s----------- \n \n ",partString);
 	for(i = 0; i< offsetBuffer; i++){
		if(part == VAR && h->varBuffer[i]) {
			printf("%s",h->varBuffer[i]);
		}
		if(part == ASSIGN && h->assignBuffer[i]) {
			printf("%s",h->assignBuffer[i]);
		}
		if(part == TRANS && h->transBuffer[i]) {
			printf("%s",h->transBuffer[i]);
		}		
  }
  printf("------------BUFFER %s----------- \n \n ",partString);
}

void printHeader(HeaderSmv* h) {

  if(h) {
		printf("------------INFO----------- \n \n ");
		printf("(%d) %s",h->type,h->moduleName);

		printf(" VAR: %d \n",h->VAR_POINTER);
	
		printf(" ASSIGN: %d \n",h->ASSIGN_POINTER);
	
		printf(" TRANS: %d \n",h->TRANS_POINTER);

		printf("------------INFO----------- \n \n ");

 	
 	}

// 	if(h->type == PORTS){
// 		printf("ports: %s\n",h->declaredPorts[0]);
// 	}

	char* tiposBuffers[] = {
    	"VAR",
    	"ASSIGN",
    	"TRANS",    	
	};
	int tamanho = sizeof(tiposBuffers)/sizeof(tiposBuffers[0]);
	int i;
 	for(i = 0; i< tamanho; i++){
		printHeaderBuffer(h,i,tiposBuffers[i]);
	}

}

void writeHeaderBuffer(HeaderSmv* h, int part, FILE* smvoutput){
	int i;
	int offsetBuffer;
	if(part == VAR){
		offsetBuffer = h->VAR_POINTER;
	}
	if(part == ASSIGN){
		offsetBuffer = h->ASSIGN_POINTER;
	}
	if(part == TRANS){
		offsetBuffer = h->TRANS_POINTER;
	}
	for(i = 0; i< offsetBuffer; i++){
		if(part == VAR && h->varBuffer[i]) {
			fprintf(smvoutput,"%s",h->varBuffer[i]);
		}
		if(part == ASSIGN && h->assignBuffer[i]) {
			fprintf(smvoutput,"%s",h->assignBuffer[i]);
		}
		if(part == TRANS && h->transBuffer[i]) {
			fprintf(smvoutput,"%s",h->transBuffer[i]);
		}	
	}
}


void writeHeader(HeaderSmv* header, FILE* smvoutput){
	int sizeBuffers[] = {header->VAR_POINTER, header->ASSIGN_POINTER,header->TRANS_POINTER};
	int tamanho = sizeof(sizeBuffers)/sizeof(sizeBuffers[0]);
	int i; 
	fprintf(smvoutput,"%s",header->moduleName);
	for(i = 0; i< tamanho; i++){
		writeHeaderBuffer(header,i,smvoutput);
	}
	fprintf(smvoutput,"%s","\n");
}


HeaderSmv* accessHeader(HeaderController* controller, smvtype cat, int indexOfHeader){
    HeaderSmv* toReturn = NULL;
    if(cat == MAIN){
        toReturn = controller->MAIN;
    }

    if(cat == AUTOMATA){
        toReturn = indexOfHeader != -1? controller->AUTOMATA_RELATED[indexOfHeader] : controller->AUTOMATA_RELATED[controller->H_AUTOMATA_CURRENT_SIZE-1];
    }
    if(cat == PORTS){
        toReturn = indexOfHeader != -1? controller->PORTS_RELATED[indexOfHeader] : controller->PORTS_RELATED[controller->H_PORTS_CURRENT_SIZE-1];
    }

    if(cat == FUNCTION_SMV){
        toReturn = indexOfHeader != -1? controller->FUNCTIONS[indexOfHeader] : controller->FUNCTIONS[controller->H_FUNCTION_CURRENT_SIZE-1];
    }

    if(!toReturn){
        fprintf(stderr, "[accessHeader] INVALID SMV TYPE FOR OPERATION");
        exit(-1);
    }
    return toReturn;

}


STable* accessSmvInfo(HeaderController* controller, smvtype cat, int SMV_INFO_ID){
    if(cat == MAIN){
        return controller->mainInfo;
    }
    if(SMV_INFO_ID > 0  && SMV_INFO_ID < controller->H_FUNCTION_CURRENT_SIZE  && cat == FUNCTION_SMV){
        return controller->functionsInfo[SMV_INFO_ID];
    }
//  if(cat == PORTS){
//        return controller->portsInfo;
//  }
    if(cat == AUTOMATA){
        return NULL; // melhorar, apesar que sabe-se que não tem operação em tabela de simbolos para automato (por enquanto)
    }
    fprintf(stderr, "[accessSmvInfo] INVALID SMV TYPE FOR OPERATION");
    exit(-1);
}

void addNewHeader(HeaderController* controller, HeaderSmv* newHeader){

    if(newHeader->type == MAIN){
        controller->MAIN = newHeader;
    }
    if(newHeader->type == AUTOMATA){
        controller->AUTOMATA_RELATED[controller->H_AUTOMATA_CURRENT_SIZE] = newHeader;
        controller->H_AUTOMATA_CURRENT_SIZE++;
    }
    if(newHeader->type == PORTS){
        controller->PORTS_RELATED[controller->H_PORTS_CURRENT_SIZE] = newHeader;
        controller->H_PORTS_CURRENT_SIZE++;
    }
    if(newHeader->type == FUNCTION_SMV ){
        controller->FUNCTIONS[controller->H_FUNCTION_CURRENT_SIZE] = newHeader;
        controller->H_FUNCTION_CURRENT_SIZE++;
    }
}

void propagParamDependence(HeaderSmv** headers, char* param, int sizeHeaders){
    int i;
    char* refOldPt;
    for(i = 0; i < sizeHeaders; i++){
        refOldPt = headers[i]->varBuffer[headers[i]->VAR_RENAME_POINTER];
        char* newDeclaration = addParams(refOldPt,param,"(",")");
        free(refOldPt);
        headers[i]->varBuffer[headers[i]->VAR_RENAME_POINTER] = newDeclaration;
    }
}

void addParamToPortsModule(HeaderController *controller, char *param, int first) {
    HeaderSmv* updated = accessHeader(controller,PORTS,0); // O PORTS MODULE É SEMPRE O PRIMEIRO
    char* newName = addParams(updated->moduleName,param,"(",")");
    free(updated->moduleName);
    updated->moduleName = newName;
    // agora deve propagar as alterações para todos os demais módulos
    // OU SEJA JÁ QUE ESTÁ CENTRALIZADO NO MAIN (posição 3 do buffer de VAR), deve-se substituir esse

    HeaderSmv* mainUpdate = accessHeader(controller,MAIN,-1);

    char* refOldPt;
    refOldPt = mainUpdate->varBuffer[mainUpdate->VAR_RENAME_POINTER];
    char* newDeclaration = addParams(refOldPt,param,"(",")");
    free(refOldPt);
    mainUpdate->varBuffer[mainUpdate->VAR_RENAME_POINTER] = newDeclaration;

//    if(!first){
//        propagParamDependence(controller->MAIN_RELATED,param,controller->H_MAIN_CURRENT_SIZE);
//    }
//    propagParamDependence(controller->AUTOMATA_RELATED,param,controller->H_AUTOMATA_CURRENT_SIZE);
}
