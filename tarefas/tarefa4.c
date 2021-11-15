// No programa cada nodo testa ate encontar o proximo nodo correto e 
// atualiza o vetor de estados com as novidades do nodo correto

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
	int *state; // vetor com o estado dos demais processos
				// nesse vetor as entradas impares indicam nodo falho,
				// enquanto as pares indicam nodo correto
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
		// --- Nomeia cada nodo
		memset(faName, '\0', 5);
		sprintf(faName, "%d", i);
		nodo[i].id = facility(faName, 1);

		// --- Aloca e inicializa o vetor state para cada nodo
		nodo[i].state = (int *) malloc(sizeof(int)*N);
		for(int j = 0; j < N; j++)
			nodo[i].state[j] = -1;
		nodo[i].state[i] = 0;
	}

	// ----- Escalonando eventos

	for(i = 0; i < N; i++){
		schedule(test, 30.0, i); // estamos escalonando testes para todos os nodos executarem no tempo 30
	}
	schedule(fault, 31.0, 1);	// o processo 1 falha no tempo 31
	schedule(repair, 61.0, 1);	// o processo 1 recupera no tempo 61
	schedule(fault, 31.0, 2);	// o processo 2 falha no tempo 31
	schedule(repair, 92.0, 2);	// o processo 2 recupera no tempo 61

	// ----- Loop principal da simulacao

	while(time() < 100.0){
		cause(&event, &token); // verifica para cada token em cada tempo se ha evento para ocorrer
		switch(event){
			case test: 
				if(status(nodo[token].id) != 0) break; // testador esta falho, nao testa

				// ----- Testando ate encontrar o proximo nodo correto

				// --- Faz o primeiro teste
				int proximoToken, resultado;
				proximoToken = token + 1;
				if(proximoToken >= N) // se esta no ultimo token volta ao comeco
					proximoToken = 0;
				resultado = status(nodo[proximoToken].id);

				// Aqui uso o valor absoluto do estado, pois estou considerando -1 tanto como desconhecido
				// como falho, e como -1 % 2 = -1 tenho que usar a funcao abs()
				if(resultado % 2 != abs(nodo[token].state[proximoToken] % 2)) 	// se a paridade do resultado for diferente da do estado
					nodo[token].state[proximoToken]++;							// atualiza a paridade

				printf("O nodo %d testou: \n", token);
				while(resultado == 1){ // enquanto nao encontrar um nodo correto
					printf("                o nodo %d %s no tempo %4.1f\n", proximoToken, nomeEstados[resultado], time());
					proximoToken++;
					if(proximoToken >= N) // se esta no ultimo token volta ao comeco
						proximoToken = 0;
					resultado = status(nodo[proximoToken].id);
					if(resultado % 2 != abs(nodo[token].state[proximoToken] % 2)) 	// se a paridade do resultado for diferente da do estado
						nodo[token].state[proximoToken]++;							// atualiza a paridade
				} // while

				// ----- Imprime o ultimo resultado e obtem os estados novos do nodo correto testado

				int tokenNodoCorreto = proximoToken;
				if(tokenNodoCorreto != token){ // caso tenha encontrado um nodo correto
					printf("                o nodo %d %s no tempo %4.1f\n", tokenNodoCorreto, nomeEstados[resultado], time());

					// --- Atualiza o vetor de estado

					proximoToken = tokenNodoCorreto + 1;
					if(proximoToken >= N) // se esta no ultimo token volta ao comeco
						proximoToken = 0;
					while(proximoToken != token){
						int novoEstado = abs(nodo[tokenNodoCorreto].state[proximoToken]);
						int antigoEstado = abs(nodo[token].state[proximoToken]);
						if(novoEstado % 2 != antigoEstado % 2) 	// se a paridade do resultado for diferente da do estado
							nodo[token].state[proximoToken]++;	// atualiza a paridade
						proximoToken++;
						if(proximoToken >= N) // se esta no ultimo token volta ao comeco
							proximoToken = 0;
					} // while
				} else // caso todos estejam falhos
					printf("O nodo %d testou todos os nodos, e eles estao todos falhos no tempo %4.1f\n", token, time());

				// ----- Imprime o vetor de estados do nodo
				
				printf("	Vetor de estados do nodo %d: ", token);
				for(i = 0; i < N; i++)
					printf("| [%d] = %d ", i, nodo[token].state[i]);
				printf("\n");
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