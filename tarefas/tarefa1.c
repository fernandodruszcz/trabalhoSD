// No programa cada nodo testa o nodo seguinte do anel

// Autores: Prof Elias e Fernando Francisco Druszcz
// Ultima modificacao: 29/10/2021

#include <stdio.h>
#include <stdlib.h>
#include "smpl.h"


// ----- Eventos

#define test 1
#define fault 2
#define repair 3

const char *nomeEstados[] = {"Correto", "Falho"}; 	// um vetor com o nome dos status
									 			   	// uso := 	estado = status();
													//			nomeEstados[estado] -> "";

// ----- Descritor do nodo SMPL

typedef struct {
	int id; // identificador facility SMPL
	// outras variaveis locais dos nodos sao declaradas aqui
} tipoNodo;

tipoNodo *nodo;

int main(int argc, char *argv[]){
	static int N, // n total de processos
			   token, // o processo que está executando em um instante de tempo 
			   event, 
			   r, i;
	static char faName[5]; // nome da facility

	if(argc != 2){
		puts("Uso correto: ./tempo <num-nodos>");
		return 1;
	}

	N = atoi(argv[1]);
	smpl(0, "Um exemplo de simulação");
	reset();
	stream(1);

	// ----- Inicializacao de variaveis

	nodo = (tipoNodo *) malloc(sizeof(tipoNodo)*N);

	for(i = 0; i < N; i++){
		memset(faName, '\0', 5);
		sprintf(faName, "%d", i);
		nodo[i].id = facility(faName, 1);
	}

	// ----- Escalonando eventos

	for(i = 0; i < N; i++){
		schedule(test, 30.0, i); // estamos escalonando testes para todos os nodos executarem no tempo 30
	}
	schedule(fault, 31.0, 1);	// o processo 1 falha no tempo 31
	schedule(repair, 61.0, 1);	// o processo 1 recupera no tempo 61

	// ----- Loop principal da simulacao

	while(time() < 100.0){
		cause(&event, &token); // verifica para cada token em cada tempo se ha evento para ocorrer
		switch(event){
			case test: 
				if(status(nodo[token].id) != 0) break; // testador esta falho, nao testa

				// ----- Testando o nodo seguinte

				int proximoToken, resultado;
				proximoToken = token + 1;
				if(proximoToken >= N) // se esta no ultimo token volta ao comeco
					proximoToken = 0;
				resultado = status(nodo[proximoToken].id);

				printf("O nodo %d testou o nodo %d %s no tempo %4.1f\n", token, proximoToken, nomeEstados[resultado], time());
				schedule(test, 30.0, token); // escalona novo teste para o tempo atual + 30
				break;
			case fault:
				r = request(nodo[token].id, token, 0);
				if(r != 0){
					puts("Não foi possível falhar o nodo");
					break;
				}
				printf("O nodo %d falhou no tempo %4.1f\n", token, time());
				break;
			case repair:
				release(nodo[token].id, token);
				printf("O nodo %d recuperou no tempo %4.1f\n", token, time());
				schedule(test, 30.0, token);
				break;
		} // switch
	} // while
	return 0;
} // main