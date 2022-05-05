#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>


/*
 nivel 1: revisar cuando la solucion entra en abiertos
 nivel 2: compararEstado
 nivel 3: no rellenar mapa, jugar con costes
 nivel 4: heuristica para encontrar los 3 objetivos simultaneamente
			maximo de las 3 distancias?¿
*/


// Este es el método principal que se piden en la practica.
// Tiene como entrada la información de los sensores y devuelve la acción a realizar.
// Para ver los distintos sensores mirar fichero "comportamiento.hpp"
Action ComportamientoJugador::think(Sensores sensores)
{

	actual.fila = sensores.posF;
	actual.columna = sensores.posC;
	actual.orientacion = sensores.sentido;

	cout << "Fila: " << actual.fila << endl;
	cout << "Col : " << actual.columna << endl;
	cout << "Ori : " << actual.orientacion << endl;

	// Capturo los destinos
	cout << "sensores.num_destinos : " << sensores.num_destinos << endl;
	objetivos.clear();
	for (int i = 0; i < sensores.num_destinos; i++)
	{
		estado aux;
		aux.fila = sensores.destino[2 * i];
		aux.columna = sensores.destino[2 * i + 1];
		objetivos.push_back(aux);
	}

	if (!hayPlan)
	{
		hayPlan = pathFinding(sensores.nivel, actual, objetivos, plan);
	}
	Action sigAccion;
	if (hayPlan && plan.size() > 0)
	{
		sigAccion = plan.front();
		plan.erase(plan.begin());
	}
	else
	{
		cout << "no se pudo encontrar plan" << endl;
	}

	return sigAccion;
}

// Llama al algoritmo de busqueda que se usara en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding(int level, const estado &origen, const list<estado> &destino, list<Action> &plan)
{
	switch (level)
	{
		estado un_objetivo;

	case 0:
		cout << "Demo\n";
		un_objetivo = objetivos.front();
		cout << "fila: " << un_objetivo.fila << " col:" << un_objetivo.columna << endl;
		return pathFinding_Profundidad(origen, un_objetivo, plan);
		break;

	case 1:
		cout << "Optimo numero de acciones\n";
		// Incluir aqui la llamada al busqueda en anchura
		un_objetivo = objetivos.front();
		cout << "fila: " << un_objetivo.fila << " col:" << un_objetivo.columna << endl;
		return pathFinding_Anchura(origen, un_objetivo, plan);
		break;
	case 2:
		cout << "Optimo en coste\n";
		// Incluir aqui la llamada al busqueda de costo uniforme/A*
		un_objetivo = objetivos.front();
		cout << "fila: " << un_objetivo.fila << " col:" << un_objetivo.columna << endl;
		return pathFinding_AEstrella(origen, un_objetivo, plan);
		cout << "No implementado aun\n";
		break;
	case 3:
		cout << "Reto 1: Descubrir el mapa\n";
		// Incluir aqui la llamada al algoritmo de busqueda para el Reto 1
		cout << "No implementado aun\n";
		break;
	case 4:
		cout << "Reto 2: Maximizar objetivos\n";
		// Incluir aqui la llamada al algoritmo de busqueda para el Reto 2
		cout << "No implementado aun\n";
		break;
	}
	return false;
}

//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el codigo en caracter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla)
{
	if (casilla == 'P' or casilla == 'M')
		return true;
	else
		return false;
}

// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st)
{
	int fil = st.fila, col = st.columna;

	// calculo cual es la casilla de delante del agente
	switch (st.orientacion)
	{
	case 0:
		fil--;
		break;
	case 1:
		fil--;
		col++;
		break;
	case 2:
		col++;
		break;
	case 3:
		fil++;
		col++;
		break;
	case 4:
		fil++;
		break;
	case 5:
		fil++;
		col--;
		break;
	case 6:
		col--;
		break;
	case 7:
		fil--;
		col--;
		break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil < 0 or fil >= mapaResultado.size())
		return true;
	if (col < 0 or col >= mapaResultado[0].size())
		return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col]))
	{
		// No hay obstaculo, actualizo el parametro st poniendo la casilla de delante.
		st.fila = fil;
		st.columna = col;
		return false;
	}
	else
	{
		return true;
	}
}


struct ComparaEstados
{
	bool operator()(const estado &a, const estado &n) const
	{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
			(a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};

struct ComparaEstadosN2
{
	bool operator()(const estado &a, const estado &n) const
	{
		bool cond1 = a.fila > n.fila;
		bool cond2 = a.fila == n.fila and a.columna > n.columna;
		bool cond3 = a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion;
		bool cond4 = a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.bikini > n.bikini;
		bool cond5 = a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.bikini == n.bikini and a.zapatillas > n.zapatillas;

		if (cond1 || cond2 || cond3 || cond4 || cond5)
			return true;
		else
			return false;
	}
};

struct ComparaHeuristica
{
	bool operator()(const nodo &a, const nodo &b) const
	{
		return a < b;
	}
};


// Implementación de la busqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan)
{
	// Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstados> Cerrados; // Lista de Cerrados
	stack<nodo> Abiertos;				  // Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.empty();

	Abiertos.push(current);

	while (!Abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{

		Abiertos.pop();
		Cerrados.insert(current.st);

		// Generar descendiente de girar a la derecha 90 grados
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
		if (Cerrados.find(hijoTurnR.st) == Cerrados.end())
		{
			hijoTurnR.secuencia.push_back(actTURN_R);
			Abiertos.push(hijoTurnR);
		}

		// Generar descendiente de girar a la derecha 45 grados
		nodo hijoSEMITurnR = current;
		hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
		if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end())
		{
			hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
			Abiertos.push(hijoSEMITurnR);
		}

		// Generar descendiente de girar a la izquierda 90 grados
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;
		if (Cerrados.find(hijoTurnL.st) == Cerrados.end())
		{
			hijoTurnL.secuencia.push_back(actTURN_L);
			Abiertos.push(hijoTurnL);
		}

		// Generar descendiente de girar a la izquierda 45 grados
		nodo hijoSEMITurnL = current;
		hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;
		if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end())
		{
			hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);
			Abiertos.push(hijoSEMITurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st))
		{
			if (Cerrados.find(hijoForward.st) == Cerrados.end())
			{
				hijoForward.secuencia.push_back(actFORWARD);
				Abiertos.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty())
		{
			current = Abiertos.top();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}

	return false;
}

// Sacar por la consola la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan)
{
	auto it = plan.begin();
	while (it != plan.end())
	{
		if (*it == actFORWARD)
		{
			cout << "A ";
		}
		else if (*it == actTURN_R)
		{
			cout << "D ";
		}
		else if (*it == actSEMITURN_R)
		{
			cout << "d ";
		}
		else if (*it == actTURN_L)
		{
			cout << "I ";
		}
		else if (*it == actSEMITURN_L)
		{
			cout << "i ";
		}
		else
		{
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}

// Funcion auxiliar para poner a 0 todas las casillas de una matriz
void AnularMatriz(vector<vector<unsigned char>> &m)
{
	for (int i = 0; i < m[0].size(); i++)
	{
		for (int j = 0; j < m.size(); j++)
		{
			m[i][j] = 0;
		}
	}
}

// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan)
{
	AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it != plan.end())
	{
		if (*it == actFORWARD)
		{
			switch (cst.orientacion)
			{
			case 0:
				cst.fila--;
				break;
			case 1:
				cst.fila--;
				cst.columna++;
				break;
			case 2:
				cst.columna++;
				break;
			case 3:
				cst.fila++;
				cst.columna++;
				break;
			case 4:
				cst.fila++;
				break;
			case 5:
				cst.fila++;
				cst.columna--;
				break;
			case 6:
				cst.columna--;
				break;
			case 7:
				cst.fila--;
				cst.columna--;
				break;
			}
			mapaConPlan[cst.fila][cst.columna] = 1;
		}
		else if (*it == actTURN_R)
		{
			cst.orientacion = (cst.orientacion + 2) % 8;
		}
		else if (*it == actSEMITURN_R)
		{
			cst.orientacion = (cst.orientacion + 1) % 8;
		}
		else if (*it == actTURN_L)
		{
			cst.orientacion = (cst.orientacion + 6) % 8;
		}
		else if (*it == actSEMITURN_L)
		{
			cst.orientacion = (cst.orientacion + 7) % 8;
		}
		it++;
	}
}

bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan)
{
	// Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstados> Cerrados; // Lista de Cerrados
	queue<nodo> Abiertos;				  // Lista de Abiertos

	nodo current;
	nodo camino;
	current.st = origen;
	current.secuencia.empty();

	Abiertos.push(current);

	bool planEncontrado = false;

	while (!Abiertos.empty() and !planEncontrado)
	{
		planEncontrado = false;

		Abiertos.pop();
		Cerrados.insert(current.st);

		// Generar descendiente de girar a la derecha 90 grados
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
		if (Cerrados.find(hijoTurnR.st) == Cerrados.end())
		{
			hijoTurnR.secuencia.push_back(actTURN_R);
			Abiertos.push(hijoTurnR);
		}

		// Generar descendiente de girar a la derecha 45 grados
		nodo hijoSEMITurnR = current;
		hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
		if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end())
		{
			hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
			Abiertos.push(hijoSEMITurnR);
		}

		// Generar descendiente de girar a la izquierda 90 grados
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;

		if (Cerrados.find(hijoTurnL.st) == Cerrados.end())
		{
			hijoTurnL.secuencia.push_back(actTURN_L);

			Abiertos.push(hijoTurnL);
		}

		// Generar descendiente de girar a la izquierda 45 grados
		nodo hijoSEMITurnL = current;
		hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;

		if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end())
		{
			hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);

			Abiertos.push(hijoSEMITurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;

		if (!HayObstaculoDelante(hijoForward.st))
		{
			if (Cerrados.find(hijoForward.st) == Cerrados.end())
			{
				if (hijoForward.st.fila == destino.fila and hijoForward.st.columna == destino.columna)
				{
					cout << "SE ENCONTRÓ CAMINOOO" << endl;
					planEncontrado = true;
					hijoForward.secuencia.push_back(actFORWARD);
					Abiertos.push(hijoForward);
					cout << "FILA HIJO " << hijoForward.st.fila << " COLUMNA HIJO " << hijoForward.st.columna <<endl;
				}
				else
				{
					hijoForward.secuencia.push_back(actFORWARD);
					Abiertos.push(hijoForward);
				}
			}
		}

		// Tomo el siguiente valor de la Abiertos
		if (planEncontrado)
		{
			current = hijoForward;
			cout << "Current Fila " << current.st.fila << " Current Columna " << current.st.columna << endl;
		}
		else if (!Abiertos.empty())
		{
			current = Abiertos.front();
			Abiertos.pop();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}

	return false;
}

int ComportamientoJugador::calcularCoste(nodo & n, const Action & accion){
	char suelo = mapaResultado[n.st.fila][n.st.columna];
	int resultado = 1;
	int indice; 

	const int costeNormalAgua [] = {200, 500, 300};
	const int costeNormalBosque [] = {100, 3, 2};
	const int costeTierra [] = {2, 2, 1};
	const int costeResto = 1;
	const int costeReducidoAgua[] = {10, 5, 2};
	const int costeReducidoBosque[] = {15, 1, 1};

	
	switch(accion){
		case actFORWARD:
			indice = 0;
		break;
		case actSEMITURN_L:
		case actSEMITURN_R:
			indice = 2; 
		break;
		case actTURN_L:
		case actTURN_R:
			indice = 1;
		break;
	}



	switch (suelo)
	{
	case 'A':
		if(n.bikini){
			resultado = costeReducidoAgua[indice];
		}else{
			resultado = costeNormalAgua[indice];
		}
		break;
	case 'B':
		if(n.zapatillas){
			resultado = costeReducidoBosque[indice];
		}else{
			resultado = costeNormalBosque[indice];
		}
		break;
	case 'T':
		resultado = costeTierra[indice];
		break;
	default:
		resultado = 1;
		break;
	}

	if(accion == actIDLE){
		resultado = 0;
	}

	return resultado;

}

int ComportamientoJugador::calcularHeuristica(nodo & n, const estado & objetivo)
{
	int difFil, difCol;

	difFil = abs(n.st.fila - objetivo.fila);
	difCol = abs(n.st.columna - objetivo.columna);

	if(difFil > difCol){
		return difFil;
	}else{
		return difCol;
	}
}

bool ComportamientoJugador::pathFinding_AEstrella(const estado &origen, const estado &destino, list<Action> &plan)
{
	// Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstadosN2> Cerrados; // Lista de Cerrados
	priority_queue<nodo, vector<nodo>, ComparaHeuristica> Abiertos;				  // Lista de Abiertos

	nodo current;

	current.st = origen;
	current.secuencia.empty();
	current.costeAcumulado = 0;
	current.st.bikini = current.st.zapatillas = false;

	current.coste = 0;
	current.heuristica = calcularHeuristica(current, destino);

	Abiertos.push(current);

	while (!Abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{
		Abiertos.pop();

		if(Cerrados.find(current.st) == Cerrados.end()){
			Cerrados.insert(current.st);

			if(mapaResultado[current.st.fila][current.st.columna] == 'K'){
				current.st.bikini = true;
				current.zapatillas = false;
			}else if(mapaResultado[current.st.fila][current.st.columna] == 'D'){
				current.st.bikini = false;
				current.st.zapatillas = true;
			}

			// Generar descendiente de girar a la derecha 90 grados
			nodo hijoTurnR = current;
			hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
			if (Cerrados.find(hijoTurnR.st) == Cerrados.end())
			{
				hijoTurnR.coste = calcularCoste(current, actTURN_R);
				hijoTurnR.costeAcumulado = hijoTurnR.coste + current.costeAcumulado;
				hijoTurnR.heuristica = calcularHeuristica(hijoTurnR, destino);
				hijoTurnR.secuencia.push_back(actTURN_R);
				Abiertos.push(hijoTurnR);
			}

			// Generar descendiente de girar a la derecha 45 grados
			nodo hijoSEMITurnR = current;
			hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
			if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end())
			{
				hijoSEMITurnR.coste = calcularCoste(current, actSEMITURN_R);
				hijoSEMITurnR.costeAcumulado = hijoSEMITurnR.coste + current.costeAcumulado;
				hijoSEMITurnR.heuristica = calcularHeuristica(current, destino);
				hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
				Abiertos.push(hijoSEMITurnR);
			}

			// Generar descendiente de girar a la izquierda 90 grados
			nodo hijoTurnL = current;
			hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;

			if (Cerrados.find(hijoTurnL.st) == Cerrados.end())
			{
				hijoTurnL.coste = calcularCoste(current, actTURN_L);
				hijoTurnL.costeAcumulado = hijoTurnL.coste + current.costeAcumulado;
				hijoTurnL.heuristica = calcularHeuristica(current, destino);
				hijoTurnL.secuencia.push_back(actTURN_L);
				Abiertos.push(hijoTurnL);
			}

			// Generar descendiente de girar a la izquierda 45 grados
			nodo hijoSEMITurnL = current;
			hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;

			if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end())
			{
				hijoSEMITurnL.coste = calcularCoste(current, actSEMITURN_L);
				hijoSEMITurnL.costeAcumulado = hijoSEMITurnL.coste + current.costeAcumulado;
				hijoSEMITurnL.heuristica = calcularHeuristica(current, destino);
				hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);
				Abiertos.push(hijoSEMITurnL);
			}	

			// Generar descendiente de avanzar
			nodo hijoForward = current;

			if (!HayObstaculoDelante(hijoForward.st))
			{
				if (Cerrados.find(hijoForward.st) == Cerrados.end())
				{
					hijoForward.coste = calcularCoste(current, actFORWARD);
					hijoForward.costeAcumulado = hijoForward.coste + current.costeAcumulado;
					hijoForward.heuristica = calcularHeuristica(hijoForward, destino);
					hijoForward.secuencia.push_back(actFORWARD);
					Abiertos.push(hijoForward);
				}
			}

		}

		// Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty())
		{
			current = Abiertos.top();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}

	return false;
}

int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}
