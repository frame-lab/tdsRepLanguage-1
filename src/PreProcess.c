#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../headers/PreProcess.h"


/*
	salva o headerSMV do módulo lido anteriormente. 
	se for um automato o ponteiro de transição não é vázio.
	efeito colateral: * ao criar ao salvar o header incrementa o tamanho de headers do HeaderController
					  *	cria um header (necessita de free depois) 
*/

void initPreProcessHeader(smvtype type, char* moduleName, HeaderController* Hcontrol) {
	int transP = type == AUTOMATA? 0 : -1;
    HeaderSmv* newHeader = createHeader(type, moduleName, 0, 0, transP);
    addNewHeader(Hcontrol,newHeader);
}


void selectBuffer(headerpart part, char* line, HeaderSmv* header, int controlRename) {
	int pt;
	int tam = strlen(line);
	char* aloc = malloc((tam+1) * sizeof(char));
	if(part != TRANS)
	{
		if(part == VAR) 
		{

            if(header->type == AUTOMATA || (header->type == MAIN && header->VAR_POINTER != header->VAR_RENAME_POINTER) ){

                char* ref = strstr(line,"(");
                if(ref){
                    ref = overwriteParam(line,"ports");
                    strcpy(aloc,ref);
                    free(ref);
                }
                else{
                    strcpy(aloc,line);
                }
            }else{
                strcpy(aloc,line);
            }
            pt = header->VAR_POINTER;
            header->varBuffer[pt] = aloc;
            header->VAR_POINTER += 1;
		}
		if(part == ASSIGN) 
		{
            strcpy(aloc,line);
			pt = header->ASSIGN_POINTER;
			header->assignBuffer[pt] = aloc;
			header->ASSIGN_POINTER += 1;
		}
	}
	else
	{
		pt = header->TRANS_POINTER;
		if(!controlRename)
		{
			strcpy(aloc,line);
			header->transBuffer[pt] = aloc;
		}
		else
		{
			//char** bufferAux = clearOldPortsRefs(line); 
			//printf("[selectBuffer] tratamento de rename refs a portsModule ANTES:%s\n\n",line);
			//strcpy(aloc,bufferAux);
			clearOldPortsRefs(line,aloc);
			header->transBuffer[pt] = aloc;
			//printf("[selectBuffer] tratamento de rename refs a portsModule DEPOIS:%s\n\n",header->transBuffer[pt]);			
			//if(bufferAux){
			//	free(bufferAux);	
			//}
		}
		header->TRANS_POINTER += 1;

	}
}

/*
	prepara para chamar a função de escolha de buffer, t
	ratando a posição (pos) que representa o tamanho do vetor de HEADERS do controller (LEN-1)
						 
*/
void saveLineOnBuffer(smvtype currentHeader, headerpart part, char *line, HeaderController *Hcontrol, int controlRename)  {
    HeaderSmv* current = accessHeader(Hcontrol,currentHeader,-1);
    selectBuffer(part, line, current, controlRename);
}

void processPhase(smvtype stage, headerpart part, HeaderController *Hcontrol, char *line, int controlRename) {

	// modulo
	if(part == CREATE_MODULE) {
		//printf("[processPhase] Start module %d %s \n",Hcontrol->CURRENT_SIZE,line);
		initPreProcessHeader(stage,line, Hcontrol);
	}
	// VAR, ASSIGN, TRANS
	else{
        saveLineOnBuffer(stage, part, line, Hcontrol, controlRename);
	}


}

void setUpMainSmvTable(HeaderController *Hcontrol, STable *global)
{
	// seta alguns pontos de interesse da diretiva temporal
	char nome[] = "time";
    char* auxDelim;
    char* auxFim;
	char* linhaLida = Hcontrol->MAIN->varBuffer[1];
	int pos = 1;
	int pointIni;
	int pointEnd;
	int tam = strlen(linhaLida);
    int min = 0;
    int max = 3;

    auxDelim = strstr(linhaLida,":");

    auxFim = strstr(auxDelim,"..");

    pointIni = (auxDelim-linhaLida+2);
    pointEnd = ((auxFim-linhaLida))-1;

    void* po[] = {&pos, &tam, &pointIni, &pointEnd, &min,&max};
    addValue(nome, po, WRITE_SMV_INFO, 6, 0, Hcontrol->mainInfo, 0);


    // remover depois? (assim como a gente deve fazer o no pré processamento o "loop do time")
	char* linhaLidaInit = Hcontrol->MAIN->assignBuffer[1];
	pos = 1;
	tam = strlen(linhaLidaInit);

	auxDelim = strstr(linhaLidaInit, "=");
	auxFim = strstr(auxDelim,";");
    pointIni = auxDelim - linhaLidaInit+2; //
    pointEnd = auxFim - linhaLidaInit-1; // entra no intervalo de sobrescrita

    addValue("init(time)", po, WRITE_SMV_INFO, 4, 0, Hcontrol->mainInfo, 0);


	char* linhaLidaNext = Hcontrol->MAIN->assignBuffer[3];
    pos = 3; // note que a posição de inicio de leitura do next é irrelevante pela formatação do case
	tam = strlen(linhaLidaNext);

    auxDelim = strstr(linhaLidaNext, "<");
    auxFim = strstr(auxDelim,":");
    pointIni = auxDelim - linhaLidaNext+2;
    pointEnd = auxFim - linhaLidaNext-1;

    addValue("next(time)", po, WRITE_SMV_INFO, 4, 0, Hcontrol->mainInfo, 0);
	// ele salva: time = 6 : 0; (reboot) ou  time < 3: time + 1; (incremento até F_TIME)
		

	// seta as diretivas temporais globais da linguagem
	char* diretivas[] = {"I_TIME", "C_TIME","F_TIME"};
	int IC = 0;
	int F = 3;
	void* valIC[] = {&IC};
	void* valF[] = {&F};
    addValue(diretivas[0], valIC, T_DIRECTIVE_ENTRY, 1, 0, global, 0);
    addValue(diretivas[1], valIC, T_DIRECTIVE_ENTRY, 1, 0, global, 0);
    addValue(diretivas[2], valF, T_DIRECTIVE_ENTRY, 1, 0, global, 0);
}


void processPorts(char* buffer, char* varString, char *fVarString, int stage, HeaderController *Hcontrol) {
    HeaderSmv* portsHeader = accessHeader(Hcontrol, stage, -1);
    int iniVar = portsHeader->VAR_POINTER == 0;
    if(iniVar &&  strstr(buffer,fVarString)) {
        // tratamento de FROZEN VAR
        selectBuffer(VAR,varString,portsHeader,0);
    }
    // é porque estamos em uma porta
    else{
//        printf("[processPorts] Salvando tabela de portas (%s) \n\n",buffer);
//        printf("[processPorts] Salvando tabela de portas (%c) \n\n",buffer[1]);
        int used = 0;
        void* po[] = {&used};
        char name[] = {buffer[1],'\0'};
        addValue(name,po,LOGICAL_ENTRY,1,0,Hcontrol->originalPorts,-1);
    }
    Hcontrol->expectedPorts++;
}

void preProcessSmv(FILE *smvP, HeaderController *Hcontrol) {
	/*Strings que são usadas para a busca no arquivo*/
	char varString[] = "VAR \n";
	char assignString[] = "ASSIGN";
	char transString[] = "TRANS";
	char fVarString[] = "FROZENVAR";
	char portsModuleString[] = "MODULE portsModule";
	char automataString[] = "MODULE finalAutomata(time)";
	char beginModuleString[] = "MODULE ";	
	
	char confirmAutomataString[] = "cs: {"; // se não encontrar isso após "começar a ler um automato" quer dizer que era um módulo qualquer
	// ATUALIZAR:  usar string "caracteristica" para detectar módulos de "função" (transform)


	/*Parâmetro a ser adicionado a PortsModule e repassado as demais funções sempre */
	char timeString[] = "time";

    /*Variáveis de controle de cursor e buffer
        -> buffer
        -> propriedades do buffer
        -> ponteiro auxiliar
    */
	
	char* buffer;
	char* bufferAux;
    size_t bufsize = 300;
   	buffer = (char *) malloc(bufsize * sizeof(char));
	//bufferAux = (char *) malloc(bufsize * sizeof(char));


    /*Variáveis de controle de leitura
        -> estamos em qual "estagio" de interesse (1 = MAIN,  2 = Módulos(automato e outros), 3 = Ports)
        -> indice baseado no estagio corrente
        -> em qual fase dos estágios estamos (-1 = lendo inicio do módulo, 0 =var ,  1 = ASSIGN, 2= TRANS  ) (note que assign e trans podem não existir )
        -> estamos lendo um ASSIGN
        -> estamos lendo TRANS (relação de transição do automato)
        -> deve-se renomear referencias a portsModule
        -> estamos lendo automato
        -> estamos lendo portsModule
        -> renomeamos portsModule
    */
	int stage = MAIN;
	int index;
	int phase = CREATE_MODULE;
	int readAssign = 0;
	int readTrans = 0; // o !readTrans mais abaixo é usado para evitar comparações desncessárias com a string gigante de transições 
	int controlRename = 0;
	int readAutomata = 0;
	int readPortsModule = 0;
	int renamePortsParam = 0;

   	while ((fgets(buffer,bufsize,smvP))) {
   		
   		bufferAux = buffer;
   		// ajuda a não rodar na primeira passada (do main)
   		if(stage != MAIN && !readPortsModule && !readAutomata){
   			if(strstr(buffer,portsModuleString)) {
   				stage = PORTS;
   				readPortsModule = 1;
   			}
   			else{
   				stage = AUTOMATA;
   				readAutomata = 1;
   			}
   		}

        // estamos ainda no meio do módulo
   		if(buffer[0] != '\n') 
   		{
   			if(phase == TRANS)
   			{
   				//printf("[preProcessSmv] Buffer pré compute Phase 3 %s \n\n",buffer);
   				readTrans = (buffer[0] != 'I')? readTrans : 0;
   				controlRename = readTrans? 1 : 0; // usado para debugar o nosso renomeador de refs
   			}

   			if(phase == ASSIGN)
   			{
   				
   				if(!readPortsModule)
   				{
   					//printf("[preProcessSmv] Buffer pré computePhase 2 %s \n\n",buffer);
   					readTrans =  (buffer[0] == 'T' && strstr(buffer,transString));
   					// na verdade entramos na fase de leitura de transições que veio LOGO DEPOIS DE ATRIBUIÇÕES
   					if(readTrans)
   					{
   						//printf("[preProcessSmv] terminou ASSIGN \n\n");
   						phase = TRANS; // mudou de fase
   					}
   				}
   				else 
   				{
   					break; // não precisa mais ler(vai ser lido da arvore)
   				}
   				

   			}
            // usa a primeira letra de cada parte de interesse primeiro para realizar curto circuito
            if(phase == VAR)
            {
                //printf("[preProcessSmv] Buffer pré computePhase1 %s \n\n",buffer);
                readAssign = (buffer[0] == 'A' && strstr(buffer,assignString));
                readTrans =  (buffer[0] == 'T' && strstr(buffer,transString));

                if(!(readAssign || readTrans))
                {
                    if(readPortsModule){
                        processPorts(buffer,varString,fVarString,stage,Hcontrol);
                    }
                }
                else
                {
                    phase = readAssign ? ASSIGN : TRANS;
                    readAssign = 0;
                }

            }
            // ignora referência a portsModule nos automatos (centralizar em main) e não salva em headers as portas originais
            if(!(bufferAux[1] == 'p' && buffer[2] =='o' && bufferAux[3] == 'r' && readAutomata) && !(phase == VAR && readPortsModule)) {
                processPhase(stage, phase, Hcontrol, bufferAux, controlRename);
            }
            if(phase == CREATE_MODULE){
                phase++;
            }
   			
   		}
   		//proximo modulo (módulos são separados por quebra de linha)
   		else
   		{
   		    if(stage == MAIN){
                HeaderSmv* current = accessHeader(Hcontrol,MAIN,-1);
                char* portCenter = "\tports: portsModule(time);\n";
                current->VAR_RENAME_POINTER = current->VAR_POINTER;
   		        selectBuffer(VAR,portCenter,current,controlRename);
   		    }
   			// reinicio
   			phase = CREATE_MODULE;
   			stage++;
   			readAutomata = 0;
   			//printf("[preProcessSmv] módulo acabou \n\n");
   		}
   	}
   	free(buffer);
    //printf("terminou! \n");
//    addParamToPortsModule(Hcontrol, "random", 1);
//    addParamToPortsModule(Hcontrol, "parametromuitograndeparaserblabla", 1);
}
