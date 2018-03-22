#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/shm.h> /* shm*  */ //memoria compartida
#include <unistd.h>

#define MEMORIARUTA "/bin/ls"			   // Ruta para la llave de la memoria - ftok
#define MEMORIAID 1						   // Id used on ftok for shmget key
#define TAMANOCOMPARTIDO sizeof(int) * 105 //Tamaño de la memoria compartida

#define Turno 1
#define EstadoJuego 2
#define Ganador 3

int MemoriaCompartida();
void TurnoOtroJugador(int jug);
void Menu();
int IngresaOpcion();
void OpcionesMenu(int opcion, bool jugador);
void Rendirse(bool jug);
void IngresarPalabra(bool jug);
void Dibujar_Tablero(bool jug);
void Buscar_Letra(bool jug);
void Comprobar_Palabra(bool jug);

int *memoria;
//Variables de Memoria compartida
key_t k_memoria; //Llave para acceder a la memoria compartida
int id_memoria;  //ID para la memoria compartida

/**Variables de locales de logica de app**/
char palabra[6];
int posiciones [5] = {0,0,0,0,0};

int main(int argc, char const *argv[])
{
	memoria = NULL;

	int ExitoMemoria = MemoriaCompartida();
	if(ExitoMemoria == -1) return -1;
	//goto fin;
	srand(time(NULL));
	bool jugador = false;
	if(memoria[0] == 0){
		jugador = true;
		memoria[0] = 1;
		memoria[EstadoJuego] = 1;
		printf("Bienvenido Jugador 1");
		memoria[Turno] = 1;
		IngresarPalabra(jugador);
	} else{
		jugador = false;
		printf("Bienvenido Jugador 2");
		IngresarPalabra(jugador);
		TurnoOtroJugador(1);
	}

	while(memoria[EstadoJuego] == 1){
		Menu();
		OpcionesMenu(IngresaOpcion(), jugador);
		if(jugador){
			memoria[Turno] = 2;
			TurnoOtroJugador(2);
		} else{
			memoria[Turno] = 1;
			TurnoOtroJugador(1);
		}
	}

	if(memoria[Ganador] == 1){
		printf("\n%s\n", "Jugador 1 haz ganado");
	} else if (memoria[Ganador] == 2){
		printf("\n%s\n", "Jugador 2 haz ganado");
	} else{
		printf("\n%s\n", "Algo trambolico paso jaja");
	}
	
	if(jugador){//fin:
		//Libera la memoria compartida
		shmdt((char *)memoria);
		shmctl(id_memoria, IPC_RMID, (struct shmid_ds *)NULL);
		printf("%s\n", "fin");
	}
	return 0;
}

void TurnoOtroJugador(int jug){
	printf("\n%s\n","esperando ...");
	while(memoria[Turno]==jug && memoria[Ganador]==0){
		usleep(1000);
	}
}

void Menu(){
	printf( "\n**************************");
	printf( "\n*  1. Buscar Letra.      *");
    printf( "\n*  2. Ingresar Palabra.  *");
    printf( "\n*  3. Rendirse.          *");
    printf( "\n**************************");
    printf( "\n\n   Introduzca opcion (1-3): ");
}

int IngresaOpcion(){
	int opcion;
	fflush( stdin );
	scanf( "%d", &opcion );
	return opcion;
}

void OpcionesMenu(int opcion, bool jug){
	Lvalidar:	
	switch(opcion){
		case 1: //Buscar Letra
			Buscar_Letra(jug);
			break;
		case 2: //Ingresar Palabra
			Comprobar_Palabra(jug);
			break;
		case 3: //Rendirse
			Rendirse(jug);
			break;
		default:
			printf( "\n   Introduzca opcion correcta xD");
			printf( "\n\n   Introduzca opcion (1-3): ");
			opcion = IngresaOpcion();
			goto Lvalidar;
	}
}

void Rendirse(bool jug){
	if(jug){//Jugador 1 se Rinde
		memoria[Ganador] = 2;
		printf("\n%s", "Jugador 1 te rendiste");
	}else{//Jugador 2 se Rinde
		memoria[Ganador] = 1;
		printf("\n%s", "Jugador 2 te rendiste");
	}
	memoria[EstadoJuego] = 0;
}

void IngresarPalabra(bool jug){
	printf("\n%s\n", "Ingrese su palabra de 4 a 5 Letras:");
	IP_inicio:
	scanf("%s", palabra);
	if(strlen(palabra) < 4 || strlen(palabra) > 5){
		printf("\n%s\n", "Ingrese una palabra de 4 o 5 Letras ;V");
		goto IP_inicio;
	}

	int tam_palabra = strlen(palabra);
	int letra = 0;
	bool bandera = false;
	int corrimiento = (jug)? 14 : 30;

	while(letra!=tam_palabra){
		int posicion = corrimiento + rand() % 16;
		bandera = false;
		int i;
		for (i = 0; i < 5; i++)
		{
			if(posiciones[i] == posicion){
				bandera = true;
				break;
			}
		}
		if(!bandera){
			posiciones[letra] = posicion;
			letra++;
		}
	}
	int i;
	for(i = 0; i < tam_palabra; i++){
		memoria[posiciones[i]] = palabra[i];
		printf("\n%s %c = %d", "Para la letra:", palabra[i], posiciones[i] - corrimiento);
	}
	printf("\n");
	corrimiento = (jug)? 4 : 9;
	for(i = 0; i < tam_palabra; i++){
		memoria[corrimiento+i] = palabra[i];
	}
	Dibujar_Tablero(jug);
}

void Dibujar_Tablero(bool jug){
	int corrimiento = (jug)? 14 : 30;
	int i, j = 0;
	for(i = corrimiento; i < corrimiento+16; i++){
		if(j == 0) printf("%s", "|");
		printf("%c|", memoria[i]);
		j++;
		if(j==4){
			printf("\n");
			j = 0;
		}
	}
}

void Buscar_Letra(bool jug){
	BL_inicio:
	printf("\n%s\n", "Ingrese un numero del 0 a 15 del tablero enemigo xD:");
	int opcion = IngresaOpcion();
	if(opcion < 0 || opcion > 15){
		printf("\n%s\n", "Ingrese un numero del 0 a 15");
		goto BL_inicio;
	}
	int corrimiento = (!jug)? 14 : 30;
	printf("\n%s %d %s: %c\n", "Valor de la celda", opcion, "es", memoria[corrimiento+opcion]);
}

void Comprobar_Palabra(bool jug){
	char palabra_in [6];
	CP_inicio:
	printf("\n%s\n", "Ingrese la palabra del jugador enemigo:");
	scanf("%s", palabra_in);
	if(strlen(palabra) < 4 || strlen(palabra) > 5){
		printf("\n%s", "Ingrese una palabra de 4 o 5 Letras ;V");
		goto CP_inicio;
	}
	char palabra_enemigo [6];
	int corrimiento = (!jug)? 4 : 9;
	int i;
	for (i = 0; i < 5; i++)
		palabra_enemigo[i] = memoria[corrimiento+i];
	palabra_enemigo[5] = '\0';
	printf("%s %s\n", "Palabra del enemigo", palabra_enemigo);
	if(strcmp(palabra_in, palabra_enemigo) == 0){
		printf("\n%s\n", "Haz ganado papu");
		memoria[Ganador] = (jug)? 1 : 2;
		memoria[EstadoJuego] = 0;
	}
}

/***
* 0 Para jugador
* 1 Para indicar el Turno
* 2 Estado Juego 1 = En curso, 0 = Finalizado
* 3 Ganador 1 = Juador 1 Gana, 2 = Jugador 2 Gana
* 4 - 8 Palabra Jugador 1
* 9 - 13 Palabra Jugador 2
* 14 - 29 Tablero Jugador 1
* 30 - 45 Tablero Jugador 2
***/

int MemoriaCompartida(){
	/***********************************************
	 * 
	 *              Memoria Compartida
	 * 
	 * ********************************************/

	//Genera la llave para usar la memoria compartida
	k_memoria = ftok(MEMORIARUTA, MEMORIAID);
	if (k_memoria == (key_t)-1)
	{
		printf("main: ftok() para la memoria compartida fracaso\n");
		return -1;
	}
	//Obtiene el id a utilizar en la memoria compartida
	id_memoria = shmget(k_memoria, TAMANOCOMPARTIDO, 0777 | IPC_CREAT);
	if (id_memoria == -1)
	{
		printf("main: shmget() fracaso %d\n", errno);
		return -1;
	}

	//Obtenemos la direccion de la memoria compartida
	memoria = (int *)shmat(id_memoria, 0, 0);
	if (memoria == NULL)
	{
		printf("main: shmat() fracaso\n");
		return -1;
	}
}
