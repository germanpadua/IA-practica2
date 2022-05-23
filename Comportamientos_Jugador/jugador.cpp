#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>
#include <iterator>
#include <random>

/*
 nivel 1: revisar cuando la solucion entra en abiertos
 nivel 2: compararEstado
 nivel 3: no rellenar mapa aletoriamente, jugar con costes
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
	if (sensores.terreno[0] == 'K')
	{
		tieneBikini = true;
		tieneZapatillas = false;
	}
	else if (sensores.terreno[0] == 'D')
	{
		tieneBikini = false;
		tieneZapatillas = true;
	}

	actual.bikini = tieneBikini;
	actual.zapatillas = tieneZapatillas;

	for (int i = 0; i < 3; i++)
		actual.objiConseguido[i] = objiConseguido[i];

	if (sensores.nivel == 4)
	{
		cout << "Fila: " << posicionF << endl;
		cout << "Col : " << posicionC << endl;
		cout << "Ori : " << brujula << endl;
		cout << "Baterías Vistas: " << baterias_vistas.size() << endl;
	}
	else
	{
		cout << "Fila: " << actual.fila << endl;
		cout << "Col : " << actual.columna << endl;
		cout << "Ori : " << actual.orientacion << endl;
		cout << "Baterías Vistas: " << baterias_vistas.size() << endl;
	}
	// Capturo los destinos
	Action sigAccion = actIDLE;

	if (sensores.nivel <= 2)
	{
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
			hayPlan = pathFinding(sensores.nivel, actual, objetivos, plan, sensores);
		}

		if (hayPlan && plan.size() > 0)
		{
			sigAccion = plan.front();
			plan.erase(plan.begin());
		}
		else
		{
			cout << "no se pudo encontrar plan" << endl;
		}
	}
	else if (sensores.nivel == 3)
	{
		rellenarMapa(sensores);
		poca_bateria = sensores.bateria <= 800;

		if (sensores.terreno[0] == 'X' && sensores.bateria < 1500)
		{
			sigAccion = actIDLE;
		}
		else
		{

			if (contador >= 100 && (!poca_bateria || baterias_vistas.size() == 0))
			{
				hayPlan = false;
				plan.clear();
			}
			if (actual.fila == objetivos.front().fila && actual.columna == objetivos.front().columna)
			{
				hayPlan = false;
				plan.clear();
			}
			if (poca_bateria && !vaABateria && baterias_vistas.size() > 0)
			{
				hayPlan = false;
				plan.clear();
			}

			if (!hayPlan)
			{
				contador = 0;

				if (poca_bateria)
				{
					if (baterias_vistas.size() > 0)
					{
						set<pair<int, int>>::iterator ptr;
						/*
						ptr = baterias_vistas.begin();

						int dist_min = calcularDistancia((*ptr).first, (*ptr).second, sensores.posC, sensores.posF);
						batPosF = (*ptr).first;
						batPosC = (*ptr).second;
						*/
						int dist_min = 1000000;
						int dist_aux;

						for (auto ptr = baterias_vistas.begin(); ptr != baterias_vistas.end(); ptr++)
						{
							dist_aux = calcularDistancia((*ptr).first, (*ptr).second, sensores.posF, sensores.posC);
							cout << "Distancia a Batería: " << dist_aux << endl;
							if (dist_aux < dist_min)
							{
								dist_min = dist_aux;
								batPosF = (*ptr).first;
								batPosC = (*ptr).second;
							}
						}

						cout << endl
							 << endl
							 << "BATERÍA POSF: " << batPosF << endl;
						cout << endl
							 << "BATERÍA POSC: " << batPosC << endl;

						objetivos.clear();
						estado aux;
						aux.fila = batPosF;
						aux.columna = batPosC;
						objetivos.push_back(aux);
						vaABateria = true;
					}
					else
					{
						pair<int, int> obj;
						obj = buscarObjetivo(sensores);
						objetivos.clear();
						estado aux;
						aux.fila = obj.first;
						aux.columna = obj.second;
						objetivos.push_back(aux);
					}
				}
				else
				{
					pair<int, int> obj;
					obj = buscarObjetivo(sensores);
					objetivos.clear();
					estado aux;
					aux.fila = obj.first;
					aux.columna = obj.second;
					objetivos.push_back(aux);
				}

				hayPlan = pathFinding(sensores.nivel, actual, objetivos, plan, sensores);
			}

			if (hayPlan && plan.size() > 0)
			{
				sigAccion = plan.front();
				plan.erase(plan.begin());
				contador++;
			}
			else
			{
				cout << "no se pudo encontrar plan" << endl;
			}
		}

		cout << "Bikini: " << tieneBikini << endl;
		cout << "Zapatillas: " << tieneZapatillas << endl;

		if ((sensores.terreno[2] == 'P' || sensores.terreno[2] == 'M' || (sensores.terreno[2] == 'A' && !tieneBikini && !vaConTodo) || (sensores.terreno[2] == 'B' && !tieneZapatillas && !vaConTodo)) && sigAccion == actFORWARD)
		{
			vaConTodo = false;
			cout << "PELIGRO " << endl;
			plan.clear();
			hayPlan = false;
			sigAccion = actIDLE;
			noPuedeIr++;
		}
		else
		{
			vaConTodo = false;
		}

		if (noPuedeIr > 3)
		{
			plan.clear();
			hayPlan = false;
			noPuedeIr = 0;
			vaConTodo = true;
		}
	}
	else if (sensores.nivel == 4)
	{
		iteracion++;

		switch (ultimaAccion)
		{
		case actFORWARD:
			switch (brujula)
			{
			case norte:
				posicionF--;
				break;
			case este:
				posicionC++;
				break;
			case sur:
				posicionF++;
				break;
			case oeste:
				posicionC--;
				break;
			case noreste:
				posicionF--;
				posicionC++;
				break;
			case noroeste:
				posicionF--;
				posicionC--;
				break;
			case suroeste:
				posicionF++;
				posicionC--;
				break;
			case sureste:
				posicionF++;
				posicionC++;
				break;
			}

			break;

		case actTURN_L:
			brujula = (brujula + 6) % 8;
			break;

		case actTURN_R:
			brujula = (brujula + 2) % 8;
			break;
		case actSEMITURN_L:
			brujula = (brujula + 7) % 8;
			break;
		case actSEMITURN_R:
			brujula = (brujula + 1) % 8;
			break;
		case actIDLE:
			break;
		case actWHEREIS:
			posicionC = sensores.posC;
			posicionF = sensores.posF;
			brujula = sensores.sentido;
			necesitaPos = false;
			break;
		}

		cout << "PPPPPPP" << endl;
		if (sensores.colision)
			cout << "SE CHOCÓ" << endl;

		cout << "OOOOOOOOOO" << endl;

		if (necesitaPos || (sensores.colision))
		{
			cout << "ME COMIO" << endl;
			ultimaAccion = actWHEREIS;
			necesitaPos = false;
			hayPlan = false;
			plan.clear();
			return ultimaAccion;
		}

		actual.fila = posicionF;
		actual.columna = posicionC;
		actual.orientacion = brujula;
		cout << "sensores.num_destinos : " << sensores.num_destinos << endl;
		cout << "porcentaje descubierto: " << porcentajeDescubierto << endl;
		objetivos.clear();

		cout << "G" << endl;

		rellenarMapa(sensores);

		cout << "H" << endl;
		poca_bateria = sensores.bateria <= 800;

		if (sensores.terreno[0] == 'X' && sensores.bateria < 1500)
		{
			sigAccion = actIDLE;
		}
		else
		{

			if (poca_bateria && !vaABateria && baterias_vistas.size() > 0)
			{
				hayPlan = false;
				plan.clear();
			}

			if (poca_bateria && baterias_vistas.size() > 0)
			{
				if (!hayPlan)
				{
					set<pair<int, int>>::iterator ptr;

					ptr = baterias_vistas.begin();

					int dist_min = calcularDistancia((*ptr).first, (*ptr).second, posicionF, posicionC);
					batPosF = (*ptr).first;
					batPosC = (*ptr).second;

					int dist_aux;

					for (ptr = baterias_vistas.begin(); ptr != baterias_vistas.end(); ptr++)
					{
						dist_aux = calcularDistancia((*ptr).first, (*ptr).second, posicionF, posicionC);

						if (dist_aux < dist_min)
						{
							dist_min = dist_aux;
							batPosF = (*ptr).first;
							batPosC = (*ptr).second;
						}
					}

					cout << endl
						 << endl
						 << "BATERÍA POSF: " << batPosF << endl;
					cout << endl
						 << "BATERÍA POSC: " << batPosC << endl;

					objetivos.clear();
					estado aux;
					aux.fila = batPosF;
					aux.columna = batPosC;
					objetivos.push_back(aux);
					vaABateria = true;

					hayPlan = pathFinding(3, actual, objetivos, plan, sensores);
				}

				if (hayPlan && plan.size() > 0)
				{
					if (sensores.superficie[2] == 'a')
					{
						sigAccion = actIDLE;
					}
					else
					{
						sigAccion = plan.front();
						ultimaAccion = sigAccion;
						plan.erase(plan.begin());
					}
					contador++;
				}
				else
				{
					cout << "no se pudo encontrar plan" << endl;
				}
			}
			else
			{
				if (vaABateria)
				{
					vaABateria = false;
					plan.clear();
					hayPlan = false;
				}

				for (int i = 0; i < sensores.num_destinos; i++)
				{
					estado aux;
					aux.fila = sensores.destino[2 * i];
					aux.columna = sensores.destino[2 * i + 1];
					objetivos.push_back(aux);
					actual.objiConseguido[i] = objiConseguido[i];
				}

				int k = 0;
				for (list<estado>::const_iterator it = objetivos.begin(); it != objetivos.end(); it++)
				{
					if (actual.fila == it->fila && actual.columna == it->columna && objiConseguido[k] == false)
					{

						cout << "OBJETIVO CONSEGUIDO" << endl;
						actual.objiConseguido[k] = true;
						objiConseguido[k] = true;
						nObjConseguidos++;
						nObjTotales++;

						if ( porcentajeDescubierto < 0.30)
						{
							hayPlan = false;
						}
					}

					k++;
				}

				if(plan.size() == 0){
					hayPlan = false;
				}

				k = 0;

				if (ultObjF == posicionF && ultObjC == posicionC)
				{
					cout << "AAAAAA" << endl;
					cout << "OBJETIVO CONSEGUIDO" << endl;

					for (int i = 0; i < 3; i++)
					{
						objiConseguido[i] = false;
						actual.objiConseguido[i] = false;
					}
					plan.clear();
					hayPlan = false;
					ultObjC = -1;
					ultObjF = -1;
					nObjConseguidos = 0;
					nObjTotales++;
				}

				if (nObjConseguidos == 2)
				{
					cout << "BBBBBBB" << endl;
					for (list<estado>::const_iterator it = objetivos.begin(); it != objetivos.end(); it++)
					{
						if (!objiConseguido[k])
						{
							ultObjF = it->fila;
							ultObjC = it->columna;
						}

						k++;
					}
					cout << ultObjF << " " << ultObjC << endl;
				}

				if (!hayPlan)
				{
					cout << "I" << endl;

					if ( porcentajeDescubierto < 0.35)
					{
						estado aux;
						k = 0;
						int distMin = 1000000;
						for (list<estado>::const_iterator it = objetivos.begin(); it != objetivos.end(); it++)
						{
							if (!objiConseguido[k] && calcularDistancia(it->fila, it->columna, posicionF, posicionC) < distMin)
							{
								cout << "LLLL" << endl;
								cout << distMin << endl;
								distMin = calcularDistancia(it->fila, it->columna, posicionF, posicionC);
								aux.fila = it->fila;
								aux.columna = it->columna;
								cout << distMin << endl;
							}

							k++;
						}
						objetivos.clear();

						objetivos.push_back(aux);
						cout << aux.fila << " " << aux.columna << endl;

						hayPlan = pathFinding(3, actual, objetivos, plan, sensores);
					}
					else
					{
						hayPlan = pathFinding(sensores.nivel, actual, objetivos, plan, sensores);
					}

					cout << "J" << endl;
				}

				if (hayPlan && plan.size() > 0)
				{
					if (sensores.superficie[2] == 'a')
					{
						sigAccion = actIDLE;
					}
					else
					{
						sigAccion = plan.front();
						ultimaAccion = sigAccion;
						plan.erase(plan.begin());
					}
					contador++;
				}
				else
				{
					cout << "no se pudo encontrar plan" << endl;
				}
			}

			if ((sensores.terreno[2] == 'P' || sensores.terreno[2] == 'M' || (sensores.terreno[2] == 'A' && !tieneBikini && !vaConTodo) || (sensores.terreno[2] == 'B' && !tieneZapatillas && !vaConTodo)) && sigAccion == actFORWARD)
			{
				plan.clear();
				hayPlan = false;
				sigAccion = actIDLE;
				noPuedeIr++;
				vaConTodo = false;

				cout << "PELIGROO" << endl;
			}
			else
			{
				noPuedeIr = 0;
				vaConTodo = false;
			}

			if (noPuedeIr > 2)
			{
				cout << "VA CON TODO" << endl;
				plan.clear();
				hayPlan = false;
				noPuedeIr = 0;
				vaConTodo = true;
			}
		}

		ultimaAccion = sigAccion;
	}

	return sigAccion;
}

// Llama al algoritmo de busqueda que se usara en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding(int level, const estado &origen, const list<estado> &destino, list<Action> &plan, Sensores sensores)
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
		break;
	case 3:
		cout << "Reto 1: Descubrir el mapa\n";
		// Incluir aqui la llamada al algoritmo de busqueda para el Reto 1
		un_objetivo = objetivos.front();
		cout << "fila: " << un_objetivo.fila << " col:" << un_objetivo.columna << endl;
		return pathFinding_AEstrellaN3(origen, un_objetivo, plan, sensores);
		cout << "No implementado aun\n";
		break;
	case 4:
		cout << "Reto 2: Maximizar objetivos\n";
		// Incluir aqui la llamada al algoritmo de busqueda para el Reto 2
		return pathFinding_AEstrellaN4(origen, destino, plan, sensores);
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

struct ComparaEstadosN4
{
	bool operator()(const estado &a, const estado &n) const
	{
		bool cond1 = a.fila > n.fila;
		bool cond2 = a.fila == n.fila and a.columna > n.columna;
		bool cond3 = a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion;
		bool cond4 = a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.bikini > n.bikini;
		bool cond5 = a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.bikini == n.bikini and a.zapatillas > n.zapatillas;
		bool cond6 = a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.bikini == n.bikini and a.zapatillas == n.zapatillas and a.objiConseguido[0] > n.objiConseguido[0];
		bool cond7 = a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.bikini == n.bikini and a.zapatillas == n.zapatillas and a.objiConseguido[0] == n.objiConseguido[0] and a.objiConseguido[1] > n.objiConseguido[1];
		bool cond8 = a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.bikini == n.bikini and a.zapatillas == n.zapatillas and a.objiConseguido[0] == n.objiConseguido[0] and a.objiConseguido[1] == n.objiConseguido[1] and a.objiConseguido[2] > n.objiConseguido[2];

		if (cond1 || cond2 || cond3 || cond4 || cond5 || cond6 || cond7 || cond8)
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
			current = Abiertos.front();
		}

		if (hijoForward.st.fila == destino.fila and hijoForward.st.columna == destino.columna)
			current = hijoForward;
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

int ComportamientoJugador::calcularCosteN3(nodo &n, const Action &accion, Sensores sensores)
{
	char suelo = mapaResultado[n.st.fila][n.st.columna];
	int resultado = 1;
	int indice;
	int i, j;

	const int costeNormalAgua[] = {200, 500, 300};
	const int costeNormalBosque[] = {100, 3, 2};
	const int costeTierra[] = {2, 2, 1};
	const int costeResto = 1;
	const int costeReducidoAgua[] = {10, 5, 2};
	const int costeReducidoBosque[] = {15, 1, 1};

	switch (accion)
	{
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
		if (n.st.bikini)
		{
			resultado = costeReducidoAgua[indice];
		}
		else
		{
			resultado = costeNormalAgua[indice];
		}
		break;
	case 'B':
		if (n.st.zapatillas)
		{
			resultado = costeReducidoBosque[indice];
		}
		else
		{
			resultado = costeNormalBosque[indice];
		}
		break;
	case 'T':
		resultado = costeTierra[indice];
		break;
	case '?':
		if (sensores.nivel == 3)
			resultado = 1;
		else
			resultado = 10;
		break;
	default:
		resultado = costeResto;
		break;
	}

	if (accion == actIDLE)
	{
		resultado = 0;
	}

	int cont;

	if (hayLoboCerca(sensores) && sensores.nivel == 4)
		switch (brujula)
		{
		case norte:
			cont = -1;
			for (int k = 1; k < 4; k++)
			{
				if (n.st.fila == (posicionF - 1) && n.st.columna == (posicionC + cont))
					return 10000;

				cont++;
			}

			cont = -2;

			for (int k = 4; k < 9; k++)
			{
				if (n.st.fila == (posicionF - 2) && n.st.columna == (posicionC + cont))
					return 10000;

				cont++;
			}

			cont = -3;

			for (int k = 9; k < 16; k++)
			{
				if (n.st.fila == (posicionF - 3) && n.st.columna == (posicionC + cont))
					return 10000;

				cont++;
			}
			break;

		case noreste:
			i = -1;
			j = 0;

			for (int k = 1; k < 4; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (j < 1)
				{
					j++;
				}
				else
				{
					i++;
				}
			}

			i = -2;
			j = 0;
			for (int k = 4; k < 9; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (j < 2)
				{
					j++;
				}
				else
				{
					i++;
				}
			}

			i = -3;
			j = 0;
			for (int k = 9; k < 16; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (j < 3)
				{
					j++;
				}
				else
				{
					i++;
				}
			}

			break;

		case este:
			cont = -1;
			for (int k = 1; k < 4; k++)
			{
				if (n.st.fila == (posicionF + cont) && n.st.columna == (posicionC + 1))
					return 10000;

				cont++;
			}

			cont = -2;

			for (int k = 4; k < 9; k++)
			{
				if (n.st.fila == (posicionF + cont) && n.st.columna == (posicionC + 2))
					return 10000;

				cont++;
			}

			cont = -3;

			for (int k = 9; k < 16; k++)
			{
				if (n.st.fila == (posicionF + cont) && n.st.columna == (posicionC + 3))
					return 10000;

				cont++;
			}
			break;

		case sureste:
			i = 0;
			j = 1;

			for (int k = 1; k < 4; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (i < 1)
				{
					i++;
				}
				else
				{
					j--;
				}
			}

			i = 0;
			j = 2;
			for (int k = 4; k < 9; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (i < 2)
				{
					i++;
				}
				else
				{
					j--;
				}
			}

			i = 0;
			j = 3;
			for (int k = 9; k < 16; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (i < 3)
				{
					i++;
				}
				else
				{
					j--;
				}
			}
			break;

		case sur:
			cont = 1;
			for (int k = 1; k < 4; k++)
			{
				if (n.st.fila == (posicionF + 1) && n.st.columna == (posicionC + cont))
					return 10000;

				cont--;
			}

			cont = 2;

			for (int k = 4; k < 9; k++)
			{
				if (n.st.fila == (posicionF + 2) && n.st.columna == (posicionC + cont))
					return 10000;

				cont--;
			}

			cont = 3;

			for (int k = 9; k < 16; k++)
			{
				if (n.st.fila == (posicionF + 3) && n.st.columna == (posicionC + cont))
					return 10000;

				cont--;
			}
			break;

		case suroeste:
			i = 1;
			j = 0;

			for (int k = 1; k < 4; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (j != -1)
				{
					j--;
				}
				else
				{
					i--;
				}
			}

			i = 2;
			j = 0;
			for (int k = 4; k < 9; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (j != -2)
				{
					j--;
				}
				else
				{
					i--;
				}
			}

			i = 3;
			j = 0;
			for (int k = 9; k < 16; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (j != -3)
				{
					j--;
				}
				else
				{
					i--;
				}
			}
			break;

		case oeste:
			cont = 1;
			for (int k = 1; k < 4; k++)
			{
				if (n.st.fila == (posicionF + cont) && n.st.columna == (posicionC - 1))
					return 10000;

				cont--;
			}

			cont = 2;

			for (int k = 4; k < 9; k++)
			{
				if (n.st.fila == (posicionF + cont) && n.st.columna == (posicionC - 2))
					return 10000;

				cont--;
			}

			cont = 3;

			for (int k = 9; k < 16; k++)
			{
				if (n.st.fila == (posicionF + cont) && n.st.columna == (posicionC - 3))
					return 10000;

				cont--;
			}
			break;

		case noroeste:
			i = 0;
			j = -1;

			for (int k = 1; k < 4; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (i != -1)
				{
					i--;
				}
				else
				{
					j++;
				}
			}

			i = 0;
			j = -2;
			for (int k = 4; k < 9; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (i != -2)
				{
					i--;
				}
				else
				{
					j++;
				}
			}

			i = 0;
			j = -3;
			for (int k = 9; k < 16; k++)
			{

				if (n.st.fila == (posicionF + i) && n.st.columna == (posicionC + j))
					return 10000;

				if (i != -3)
				{
					i--;
				}
				else
				{
					j++;
				}
			}
			break;

		default:
			break;
		}

	return resultado;
}

int ComportamientoJugador::calcularCoste(nodo &n, const Action &accion)
{
	char suelo = mapaResultado[n.st.fila][n.st.columna];
	int resultado = 1;
	int indice;

	const int costeNormalAgua[] = {200, 500, 300};
	const int costeNormalBosque[] = {100, 3, 2};
	const int costeTierra[] = {2, 2, 1};
	const int costeResto = 1;
	const int costeReducidoAgua[] = {10, 5, 2};
	const int costeReducidoBosque[] = {15, 1, 1};

	switch (accion)
	{
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
		if (n.st.bikini)
		{
			resultado = costeReducidoAgua[indice];
		}
		else
		{
			resultado = costeNormalAgua[indice];
		}
		break;
	case 'B':
		if (n.st.zapatillas)
		{
			resultado = costeReducidoBosque[indice];
		}
		else
		{
			resultado = costeNormalBosque[indice];
		}
		break;
	case 'T':
		resultado = costeTierra[indice];
		break;

	default:
		resultado = costeResto;
		break;
	}

	if (accion == actIDLE)
	{
		resultado = 0;
	}

	return resultado;
}

int ComportamientoJugador::calcularHeuristica(nodo &n, const estado &objetivo)
{
	int difFil, difCol;

	difFil = abs(n.st.fila - objetivo.fila);
	difCol = abs(n.st.columna - objetivo.columna);

	if (difFil > difCol)
	{
		return difFil;
	}
	else
	{
		return difCol;
	}
}

int ComportamientoJugador::calcularHeuristicaN4(nodo &n, const list<estado> &objetivos)
{
	int difFil[3] = {0, 0, 0};
	int difCol[3] = {0, 0, 0};
	int i = 0;
	int max = 0;
	for (list<estado>::const_iterator it = objetivos.begin(); it != objetivos.end(); it++)
	{
		difFil[i] = abs(n.st.fila - it->fila);
		difCol[i] = abs(n.st.columna - it->columna);
		if (n.st.objiConseguido[i])
		{
			difFil[i] = 0;
			difCol[i] = 0;
		}

		i++;
	}

	for (int j = 0; j < 3; j++)
	{
		if (difFil[j] > max)
		{
			max = difFil[j];
		}
	}

	for (int j = 0; j < 3; j++)
	{
		if (difCol[j] > max)
		{
			max = difCol[j];
		}
	}

	return max;
}

bool ComportamientoJugador::pathFinding_AEstrella(const estado &origen, const estado &destino, list<Action> &plan)
{
	// Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstadosN2> Cerrados;							// Lista de Cerrados
	priority_queue<nodo, vector<nodo>, ComparaHeuristica> Abiertos; // Lista de Abiertos

	nodo current;

	current.st = origen;
	current.secuencia.empty();
	current.costeAcumulado = 0;

	current.coste = 0;
	current.heuristica = calcularHeuristica(current, destino);
	current.st.bikini = tieneBikini;
	current.st.zapatillas = tieneZapatillas;

	Abiertos.push(current);

	while (!Abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{
		Abiertos.pop();

		if (Cerrados.find(current.st) == Cerrados.end())
		{
			Cerrados.insert(current.st);

			if (mapaResultado[current.st.fila][current.st.columna] == 'K')
			{
				current.st.bikini = true;
				current.st.zapatillas = false;
			}
			else if (mapaResultado[current.st.fila][current.st.columna] == 'D')
			{
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

bool ComportamientoJugador::pathFinding_AEstrellaN3(const estado &origen, const estado &destino, list<Action> &plan, Sensores sensores)
{
	// Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstadosN2> Cerrados;							// Lista de Cerrados
	priority_queue<nodo, vector<nodo>, ComparaHeuristica> Abiertos; // Lista de Abiertos

	nodo current;

	current.st = origen;
	current.secuencia.empty();
	current.costeAcumulado = 0;

	current.coste = 0;
	current.heuristica = calcularHeuristica(current, destino);
	current.st.bikini = tieneBikini;
	current.st.zapatillas = tieneZapatillas;

	Abiertos.push(current);

	while (!Abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{
		Abiertos.pop();

		if (Cerrados.find(current.st) == Cerrados.end())
		{
			Cerrados.insert(current.st);

			if (mapaResultado[current.st.fila][current.st.columna] == 'K')
			{
				current.st.bikini = true;
				current.st.zapatillas = false;
			}
			else if (mapaResultado[current.st.fila][current.st.columna] == 'D')
			{
				current.st.bikini = false;
				current.st.zapatillas = true;
			}

			// Generar descendiente de girar a la derecha 90 grados
			nodo hijoTurnR = current;
			hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
			if (Cerrados.find(hijoTurnR.st) == Cerrados.end())
			{
				hijoTurnR.coste = calcularCosteN3(current, actTURN_R, sensores);
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
				hijoSEMITurnR.coste = calcularCosteN3(current, actSEMITURN_R, sensores);
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
				hijoTurnL.coste = calcularCosteN3(current, actTURN_L, sensores);
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
				hijoSEMITurnL.coste = calcularCosteN3(current, actSEMITURN_L, sensores);
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
					hijoForward.coste = calcularCosteN3(current, actFORWARD, sensores);
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

void ComportamientoJugador::rellenarMapa(Sensores sensores)
{
	int posF = sensores.posF;
	int posC = sensores.posC;
	int orientacion = sensores.sentido;

	if (sensores.nivel == 4)
	{
		posF = posicionF;
		posC = posicionC;
		orientacion = brujula;
	}
	cout << posF << endl;
	cout << posC << endl;
	cout << orientacion << endl;

	int cont = 0;
	int i, j;

	if (mapaResultado[posF][posC] == '?')
	{
		mapaResultado[posF][posC] = sensores.terreno[0];
	}
	mapaContador[posF][posC] = mapaContador[posF][posC] + 5;

	switch (orientacion)
	{
	case norte:
		cont = -1;
		for (int k = 1; k < 4; k++)
		{
			if (mapaResultado[posF - 1][posC + cont] == '?')
			{
				mapaResultado[posF - 1][posC + cont] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF - 1][posC + cont] == 'X')
			{
				pair<int, int> bateria = {posF - 1, posC + cont};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF - 1][posC + cont]++;

			cont++;
		}

		cont = -2;

		for (int k = 4; k < 9; k++)
		{
			if (mapaResultado[posF - 2][posC + cont] == '?')
			{
				mapaResultado[posF - 2][posC + cont] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF - 2][posC + cont] == 'X')
			{
				pair<int, int> bateria = {posF - 2, posC + cont};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF - 2][posC + cont]++;

			cont++;
		}

		cont = -3;

		for (int k = 9; k < 16; k++)
		{
			if (mapaResultado[posF - 3][posC + cont] == '?')
			{
				mapaResultado[posF - 3][posC + cont] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF - 3][posC + cont] == 'X')
			{
				pair<int, int> bateria = {posF - 3, posC + cont};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF - 3][posC + cont]++;

			cont++;
		}
		break;

	case noreste:
		i = -1;
		j = 0;

		for (int k = 1; k < 4; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (j < 1)
			{
				j++;
			}
			else
			{
				i++;
			}
		}

		i = -2;
		j = 0;
		for (int k = 4; k < 9; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (j < 2)
			{
				j++;
			}
			else
			{
				i++;
			}
		}

		i = -3;
		j = 0;
		for (int k = 9; k < 16; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (j < 3)
			{
				j++;
			}
			else
			{
				i++;
			}
		}

		break;

	case este:
		cont = -1;
		for (int k = 1; k < 4; k++)
		{
			if (mapaResultado[posF + cont][posC + 1] == '?')
			{
				mapaResultado[posF + cont][posC + 1] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + cont][posC + 1] == 'X')
			{
				pair<int, int> bateria = {posF + cont, posC + 1};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + cont][posC + 1]++;

			cont++;
		}

		cont = -2;

		for (int k = 4; k < 9; k++)
		{
			if (mapaResultado[posF + cont][posC + 2] == '?')
			{
				mapaResultado[posF + cont][posC + 2] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + cont][posC + 2] == 'X')
			{
				pair<int, int> bateria = {posF + cont, posC + 2};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + cont][posC + 2]++;

			cont++;
		}

		cont = -3;

		for (int k = 9; k < 16; k++)
		{
			if (mapaResultado[posF + cont][posC + 3] == '?')
			{
				mapaResultado[posF + cont][posC + 3] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + cont][posC + 3] == 'X')
			{
				pair<int, int> bateria = {posF + cont, posC + 3};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + cont][posC + 1]++;

			cont++;
		}
		break;

	case sureste:
		i = 0;
		j = 1;

		for (int k = 1; k < 4; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (i < 1)
			{
				i++;
			}
			else
			{
				j--;
			}
		}

		i = 0;
		j = 2;
		for (int k = 4; k < 9; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (i < 2)
			{
				i++;
			}
			else
			{
				j--;
			}
		}

		i = 0;
		j = 3;
		for (int k = 9; k < 16; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (i < 3)
			{
				i++;
			}
			else
			{
				j--;
			}
		}
		break;

	case sur:
		cont = 1;
		for (int k = 1; k < 4; k++)
		{
			if (mapaResultado[posF + 1][posC + cont] == '?')
			{
				mapaResultado[posF + 1][posC + cont] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + 1][posC + cont] == 'X')
			{
				pair<int, int> bateria = {posF + 1, posC + cont};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + 1][posC + cont]++;

			cont--;
		}

		cont = 2;

		for (int k = 4; k < 9; k++)
		{
			if (mapaResultado[posF + 2][posC + cont] == '?')
			{
				mapaResultado[posF + 2][posC + cont] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + 2][posC + cont] == 'X')
			{
				pair<int, int> bateria = {posF + 2, posC + cont};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + 2][posC + cont]++;

			cont--;
		}

		cont = 3;

		for (int k = 9; k < 16; k++)
		{
			if (mapaResultado[posF + 3][posC + cont] == '?')
			{
				mapaResultado[posF + 3][posC + cont] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + 3][posC + cont] == 'X')
			{
				pair<int, int> bateria = {posF + 3, posC + cont};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + 3][posC + cont]++;

			cont--;
		}
		break;

	case suroeste:
		i = 1;
		j = 0;

		for (int k = 1; k < 4; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (j != -1)
			{
				j--;
			}
			else
			{
				i--;
			}
		}

		i = 2;
		j = 0;
		for (int k = 4; k < 9; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (j != -2)
			{
				j--;
			}
			else
			{
				i--;
			}
		}

		i = 3;
		j = 0;
		for (int k = 9; k < 16; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (j != -3)
			{
				j--;
			}
			else
			{
				i--;
			}
		}
		break;

	case oeste:
		cont = 1;
		for (int k = 1; k < 4; k++)
		{
			if (mapaResultado[posF + cont][posC - 1] == '?')
			{
				mapaResultado[posF + cont][posC - 1] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + cont][posC - 1] == 'X')
			{
				pair<int, int> bateria = {posF + cont, posC - 1};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + cont][posC - 1]++;

			cont--;
		}

		cont = 2;

		for (int k = 4; k < 9; k++)
		{
			if (mapaResultado[posF + cont][posC - 2] == '?')
			{
				mapaResultado[posF + cont][posC - 2] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + cont][posC - 2] == 'X')
			{
				pair<int, int> bateria = {posF + cont, posC - 2};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + cont][posC - 2]++;

			cont--;
		}

		cont = 3;

		for (int k = 9; k < 16; k++)
		{
			if (mapaResultado[posF + cont][posC - 3] == '?')
			{
				mapaResultado[posF + cont][posC - 3] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + cont][posC - 3] == 'X')
			{
				pair<int, int> bateria = {posF + cont, posC - 3};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + cont][posC - 3]++;

			cont--;
		}
		break;

	case noroeste:
		i = 0;
		j = -1;

		for (int k = 1; k < 4; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (i != -1)
			{
				i--;
			}
			else
			{
				j++;
			}
		}

		i = 0;
		j = -2;
		for (int k = 4; k < 9; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (i != -2)
			{
				i--;
			}
			else
			{
				j++;
			}
		}

		i = 0;
		j = -3;
		for (int k = 9; k < 16; k++)
		{

			if (mapaResultado[posF + i][posC + j] == '?')
			{
				mapaResultado[posF + i][posC + j] = sensores.terreno[k];
				porcentajeDescubierto = porcentajeDescubierto + 1.0 / (mapaResultado.size() * mapaResultado.size());
			}
			if (mapaResultado[posF + i][posC + j] == 'X')
			{
				pair<int, int> bateria = {posF + i, posC + j};
				baterias_vistas.insert(bateria);
			}

			mapaContador[posF + i][posC + j]++;

			if (i != -3)
			{
				i--;
			}
			else
			{
				j++;
			}
		}
		break;

	default:
		break;
	}
}

int ComportamientoJugador::calcularDistancia(int x1, int y1, int x2, int y2)
{
	int difFil = abs(y1 - y2);
	int difCol = abs(x1 - x2);

	if (difFil > difCol)
	{
		return difFil;
	}
	else
	{
		return difCol;
	}
}

pair<int, int> ComportamientoJugador::buscarObjetivo(Sensores sensores)
{
	float min = 1000000;
	int posF, posC;
	posF = posC = -1;

	cout << "A" << endl;

	/*
	for (int i = -20; i <= 20; i ++)
	{
		for (int j = -20; j <= 20; j ++)
		{
			if (!(i== 0 && j == 0))
			{
					cout << "B " << sensores.posF + i << " " << sensores.posC + j<< endl;

				if ((sensores.posF + i) >= 0 && (sensores.posF + i) < mapaResultado.size() && (sensores.posC + j) >= 0 && (sensores.posC + j) < mapaResultado.size())
				{
					cout << "C " << sensores.posF + i << " " << sensores.posC + j<< endl;
					if (mapaContador[sensores.posF + i][sensores.posC + j] < min && mapaResultado[sensores.posF + i][sensores.posC + j] != 'P' && mapaResultado[sensores.posF + i][sensores.posC + j] != 'M' && mapaResultado[sensores.posF + i][sensores.posC + j] != '?')
					{
						cout << "D" << endl;
						min = mapaContador[sensores.posF + i][sensores.posC + j];
						posF = sensores.posF + i;
						posC = sensores.posC + j;
					}
				}
			}
		}

	}
	*/

	bool encontrado = false;
	int cont = 8;

	cout << "Porcentaje Cerca: " << porcentajeCerca(sensores) << endl;

	/*


	float porcentajeNoroeste = 0;
	float porcentajeNoreste = 0;
	float porcentajeSuroeste = 0;
	float porcentajeSureste = 0;

	for(int i=0; i < mapaResultado.size()/2; i++)
		for(int j=0; j < mapaResultado.size()/2; j++)
			if(mapaResultado[i][j] != '?')
				porcentajeNoroeste = porcentajeNoroeste + 1.0 / (mapaResultado.size()*mapaResultado.size()/4.0);

	for(int i=mapaResultado.size()/2; i < mapaResultado.size(); i++)
		for(int j=0; j < mapaResultado.size()/2; j++)
			if(mapaResultado[i][j] != '?')
				porcentajeSuroeste = porcentajeSuroeste + 1.0 / (mapaResultado.size()*mapaResultado.size()/4.0);

	for(int i=0; i < mapaResultado.size()/2; i++)
		for(int j=mapaResultado.size()/2; j < mapaResultado.size(); j++)
			if(mapaResultado[i][j] != '?')
				porcentajeNoreste = porcentajeNoreste + 1.0 / (mapaResultado.size()*mapaResultado.size()/4.0);

	for(int i=mapaResultado.size()/2; i < mapaResultado.size(); i++)
		for(int j=mapaResultado.size()/2; j < mapaResultado.size(); j++)
			if(mapaResultado[i][j] != '?')
				porcentajeSureste = porcentajeSureste + 1.0 / (mapaResultado.size()*mapaResultado.size()/4.0);

	min = 10000;

	cout << porcentajeNoreste << " " << porcentajeNoroeste << " " << porcentajeSureste << " " << porcentajeSuroeste << endl;

	if(porcentajeNoreste < min){
		min = porcentajeNoreste;
	}
	if(porcentajeNoroeste < min)
		min = porcentajeNoroeste;
	if(porcentajeSureste < min)
		min = porcentajeSureste;
	if(porcentajeSuroeste < min)
		min = porcentajeSuroeste;

	cout << min << endl;

	if(min == porcentajeNoreste){
		int rand1 = rand()%(mapaResultado.size()/2);
		int rand2 = rand()%(mapaResultado.size()/2) + mapaResultado.size()/2;
		posF = rand1;
		posC = rand2;
	}else if(min == porcentajeNoroeste){
		int rand1 = rand()%(mapaResultado.size()/2);
		int rand2 = rand()%(mapaResultado.size()/2);
		posF = rand1;
		posC = rand2;

	}else if(min == porcentajeSureste){
		int rand1 = rand()%(mapaResultado.size()/2) + mapaResultado.size()/2;
		int rand2 = rand()%(mapaResultado.size()/2) + mapaResultado.size()/2;
		posF = rand1;
		posC = rand2;

	}else if(min == porcentajeSuroeste){
		int rand1 = rand()%(mapaResultado.size()/2) + mapaResultado.size()/2;
		int rand2 = rand()%(mapaResultado.size()/2);
		posF = rand1;
		posC = rand2;

	}
	*/

	if (porcentajeCerca(sensores) > 85)
	{
		cont = mapaResultado.size() / 3;
		float porcentajeNoroeste = 0;
		float porcentajeNoreste = 0;
		float porcentajeSuroeste = 0;
		float porcentajeSureste = 0;

		for (int i = 0; i < mapaResultado.size() / 2; i++)
			for (int j = 0; j < mapaResultado.size() / 2; j++)
				if (mapaResultado[i][j] != '?')
					porcentajeNoroeste = porcentajeNoroeste + 1.0 / (mapaResultado.size() * mapaResultado.size() / 4.0);

		for (int i = mapaResultado.size() / 2; i < mapaResultado.size(); i++)
			for (int j = 0; j < mapaResultado.size() / 2; j++)
				if (mapaResultado[i][j] != '?')
					porcentajeSuroeste = porcentajeSuroeste + 1.0 / (mapaResultado.size() * mapaResultado.size() / 4.0);

		for (int i = 0; i < mapaResultado.size() / 2; i++)
			for (int j = mapaResultado.size() / 2; j < mapaResultado.size(); j++)
				if (mapaResultado[i][j] != '?')
					porcentajeNoreste = porcentajeNoreste + 1.0 / (mapaResultado.size() * mapaResultado.size() / 4.0);

		for (int i = mapaResultado.size() / 2; i < mapaResultado.size(); i++)
			for (int j = mapaResultado.size() / 2; j < mapaResultado.size(); j++)
				if (mapaResultado[i][j] != '?')
					porcentajeSureste = porcentajeSureste + 1.0 / (mapaResultado.size() * mapaResultado.size() / 4.0);

		min = 10000;

		cout << porcentajeNoreste << " " << porcentajeNoroeste << " " << porcentajeSureste << " " << porcentajeSuroeste << endl;

		if (porcentajeNoreste < min)
		{
			min = porcentajeNoreste;
		}
		if (porcentajeNoroeste < min)
			min = porcentajeNoroeste;
		if (porcentajeSureste < min)
			min = porcentajeSureste;
		if (porcentajeSuroeste < min)
			min = porcentajeSuroeste;

		cout << min << endl;

		if (min == porcentajeNoroeste)
		{
			for (int i = 3; i < mapaResultado.size() / 2; i++)
				for (int j = 3; j < mapaResultado.size() / 2; j++)
					if (mapaResultado[i][j] == '?' && mapaIntentos[i][j] < 3)
					{
						posF = i;
						posC = j;
					}
		}
		else if (min == porcentajeNoreste)
		{
			for (int i = 3; i < mapaResultado.size() / 2; i++)
				for (int j = mapaResultado.size() / 2; j < (mapaResultado.size() - 3); j++)
					if (mapaResultado[i][j] == '?' && mapaIntentos[i][j] < 3)
					{
						posF = i;
						posC = j;
					}
		}
		else if (min == porcentajeSureste)
		{
			for (int i = mapaResultado.size() / 2; i < (mapaResultado.size() - 3); i++)
				for (int j = mapaResultado.size() / 2; j < (mapaResultado.size() - 3); j++)
					if (mapaResultado[i][j] == '?' && mapaIntentos[i][j] < 3)
					{
						posF = i;
						posC = j;
					}
		}
		else if (min == porcentajeSuroeste)
		{
			for (int i = mapaResultado.size() / 2; i < (mapaResultado.size() - 3); i++)
				for (int j = 3; j < mapaResultado.size() / 2; j++)
					if (mapaResultado[i][j] == '?' && mapaIntentos[i][j] < 3)
					{
						posF = i;
						posC = j;
					}
		}
	}
	else
	{

		while (!encontrado)
		{
			if ((sensores.posF + cont) >= 3 && (sensores.posF + cont) < (mapaResultado.size() - 3))
			{
				if (sensores.sentido == sur && mapaResultado[sensores.posF + cont][sensores.posC] == '?' && mapaIntentos[sensores.posF + cont][sensores.posC] < 3)
				{
					cout << "RRRRRRR" << endl;
					encontrado = true;
					posF = sensores.posF + cont;
					posC = sensores.posC;
				}
			}
			else if ((sensores.posF - cont) >= 3 && (sensores.posF - cont) < (mapaResultado.size() - 3))
			{
				if (sensores.sentido == norte && mapaResultado[sensores.posF - cont][sensores.posC] == '?' && mapaIntentos[sensores.posF - cont][sensores.posC] < 3)
				{
					encontrado = true;
					posF = sensores.posF - cont;
					posC = sensores.posC;
				}
			}
			else if ((sensores.posC + cont) >= 3 && (sensores.posC + cont) < (mapaResultado.size() - 3))
			{
				if (sensores.sentido == este && mapaResultado[sensores.posF][sensores.posC + cont] == '?' && mapaIntentos[sensores.posF][sensores.posC + cont] < 3)
				{
					encontrado = true;
					posF = sensores.posF;
					posC = sensores.posC + cont;
				}
			}
			else if ((sensores.posC - cont) >= 3 && (sensores.posC - cont) < (mapaResultado.size() - 3))
			{
				if (sensores.sentido == oeste && mapaResultado[sensores.posF][sensores.posC - cont] == '?' && mapaIntentos[sensores.posF][sensores.posC - cont] < 3)
				{
					encontrado = true;
					posF = sensores.posF;
					posC = sensores.posC - cont;
				}
			}
			else if ((sensores.posF + cont) >= 3 && (sensores.posF + cont) < (mapaResultado.size() - 3) && (sensores.posC + cont) >= 0 && (sensores.posC + cont) < (mapaResultado.size() - 3))
			{
				if (sensores.sentido == sureste && mapaResultado[sensores.posF + cont][sensores.posC + cont] == '?' && mapaIntentos[sensores.posF + cont][sensores.posC + cont] < 3)
				{
					encontrado = true;
					posF = sensores.posF + cont;
					posC = sensores.posC + cont;
				}
			}
			else if ((sensores.posF + cont) >= 3 && (sensores.posF + cont) < (mapaResultado.size() - 3) && (sensores.posC - cont) >= 0 && (sensores.posC - cont) < (mapaResultado.size() - 3))
			{
				if (sensores.sentido == suroeste && mapaResultado[sensores.posF + cont][sensores.posC - cont] == '?' && mapaIntentos[sensores.posF + cont][sensores.posC - cont] < 3)
				{
					encontrado = true;
					posF = sensores.posF + cont;
					posC = sensores.posC - cont;
				}
			}
			else if ((sensores.posF - cont) >= 3 && (sensores.posF - cont) < (mapaResultado.size() - 3) && (sensores.posC - cont) >= 0 && (sensores.posC - cont) < (mapaResultado.size() - 3))
			{
				if (sensores.sentido == noroeste && mapaResultado[sensores.posF - cont][sensores.posC - cont] == '?' && mapaIntentos[sensores.posF - cont][sensores.posC - cont] < 3)
				{
					encontrado = true;
					posF = sensores.posF - cont;
					posC = sensores.posC - cont;
				}
			}
			else if ((sensores.posF - cont) >= 3 && (sensores.posF - cont) < (mapaResultado.size() - 3) && (sensores.posC + cont) >= 0 && (sensores.posC + cont) < (mapaResultado.size() - 3))
			{
				if (sensores.sentido == noreste && mapaResultado[sensores.posF - cont][sensores.posC + cont] == '?' && mapaIntentos[sensores.posF - cont][sensores.posC + cont] < 3)
				{
					encontrado = true;
					posF = sensores.posF - cont;
					posC = sensores.posC + cont;
				}
			}

			for (int j = sensores.posC - cont; (j <= sensores.posC + cont) && !encontrado; j++)
			{
				if (j >= 3 && (sensores.posF + cont) >= 3 && j < (mapaResultado.size() - 3) && (sensores.posF + cont) < (mapaResultado.size() - 3))
				{
					if (mapaResultado[sensores.posF + cont][j] == '?' && mapaIntentos[sensores.posF + cont][j] < 3)
					{
						cout << "TTTTTTTTTTT" << endl;
						encontrado = true;
						posF = sensores.posF + cont;
						posC = j;
					}
				}

				if (j >= 3 && (sensores.posF - cont) >= 3 && j < (mapaResultado.size() - 3) && (sensores.posF - cont) < (mapaResultado.size() - 3))
				{
					if (mapaResultado[sensores.posF - cont][j] == '?' && mapaIntentos[sensores.posF - cont][j] < 3)
					{
						encontrado = true;
						posF = sensores.posF - cont;
						posC = j;
					}
				}
			}

			for (int i = sensores.posF - cont; (i <= sensores.posF + cont) && !encontrado; i++)
			{
				if (i >= 3 && (sensores.posC + cont) >= 3 && i < (mapaResultado.size() - 3) && (sensores.posC + cont) < (mapaResultado.size() - 3))
				{
					if (mapaResultado[i][sensores.posC + cont] == '?' && mapaIntentos[i][sensores.posC + cont] < 3)
					{
						encontrado = true;
						posF = i;
						posC = sensores.posC + cont;
					}
				}

				if (i >= 3 && (sensores.posC - cont) >= 3 && i < (mapaResultado.size() - 3) && (sensores.posC - cont) < (mapaResultado.size() - 3))
				{
					if (mapaResultado[i][sensores.posC - cont] == '?' && mapaIntentos[i][sensores.posC - cont] < 3)
					{
						encontrado = true;
						posF = i;
						posC = sensores.posC - cont;
					}
				}
			}
			cont++;

			if (cont > 100)
				break;
		}
	}

	if (cont > 100)
	{
		for (int i = -20; i <= 20; i++)
		{
			for (int j = -20; j <= 20; j++)
			{
				if (!(i == 0 && j == 0))
				{
					cout << "B " << sensores.posF + i << " " << sensores.posC + j << endl;

					if ((sensores.posF + i) >= 0 && (sensores.posF + i) < mapaResultado.size() && (sensores.posC + j) >= 0 && (sensores.posC + j) < mapaResultado.size())
					{
						cout << "C " << sensores.posF + i << " " << sensores.posC + j << endl;
						if ((mapaIntentos[sensores.posF + i][sensores.posC + j] < 3) && mapaContador[sensores.posF + i][sensores.posC + j] < min && mapaResultado[sensores.posF + i][sensores.posC + j] != 'P' && mapaResultado[sensores.posF + i][sensores.posC + j] != 'M' && mapaResultado[sensores.posF + i][sensores.posC + j] != '?')
						{
							cout << "D" << endl;
							min = mapaContador[sensores.posF + i][sensores.posC + j];
							posF = sensores.posF + i;
							posC = sensores.posC + j;
						}
					}
				}
			}
		}
	}

	if (posF == -1 || posC == -1)
	{
		min = 10000000;
		for (int i = -20; i <= 20; i++)
		{
			for (int j = -20; j <= 20; j++)
			{
				if (!(i == 0 && j == 0))
				{
					cout << "B " << sensores.posF + i << " " << sensores.posC + j << endl;

					if ((sensores.posF + i) >= 0 && (sensores.posF + i) < mapaResultado.size() && (sensores.posC + j) >= 0 && (sensores.posC + j) < mapaResultado.size())
					{
						cout << "C " << sensores.posF + i << " " << sensores.posC + j << endl;
						if (mapaContador[sensores.posF + i][sensores.posC + j] < min && mapaResultado[sensores.posF + i][sensores.posC + j] != 'P' && mapaResultado[sensores.posF + i][sensores.posC + j] != 'M' && mapaResultado[sensores.posF + i][sensores.posC + j] != '?')
						{
							cout << "D" << endl;
							min = mapaContador[sensores.posF + i][sensores.posC + j];
							posF = sensores.posF + i;
							posC = sensores.posC + j;
						}
					}
				}
			}
		}
	}
	cout << "OBJETIVO POSF: " << posF << "  OBJETIVO POSC: " << posC << endl;

	mapaIntentos[posF][posC]++;

	return {posF, posC};
}

float ComportamientoJugador::porcentajeCerca(Sensores sensores)
{
	float porc = 0.0;
	float total = 0.0;
	float resultado = 0;
	for (int i = -10; i < (sensores.posF + 10); i++)
	{
		for (int j = -10; j < (sensores.posC + 10); j++)
		{

			if (i >= 0 && i < mapaResultado.size() && j >= 0 && j < mapaResultado.size())
			{
				if (mapaResultado[i][j] != '?')
				{

					porc++;
				}
				total++;
			}
		}
	}
	resultado = porc / total;

	if (total < 150)
	{
		return 0;
	}
	else
		return 100 * (resultado);
}

bool ComportamientoJugador::pathFinding_AEstrellaN4(const estado &origen, const list<estado> &destinos, list<Action> &plan, Sensores sensores)
{
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstadosN4> Cerrados;							// Lista de Cerrados
	priority_queue<nodo, vector<nodo>, ComparaHeuristica> Abiertos; // Lista de Abiertos

	nodo current;

	current.st = origen;
	current.secuencia.empty();
	current.costeAcumulado = 0;

	current.coste = 0;
	current.heuristica = calcularHeuristicaN4(current, destinos);
	current.heuristica = 0;
	current.st.bikini = tieneBikini;
	current.st.zapatillas = tieneZapatillas;

	for (int i = 0; i < 3; i++)
		current.st.objiConseguido[i] = origen.objiConseguido[i];

	Abiertos.push(current);

	while (!Abiertos.empty() and !((current.st.objiConseguido[0] == true) and (current.st.objiConseguido[1] == true) and (current.st.objiConseguido[2] == true)))
	{
		Abiertos.pop();

		if (Cerrados.find(current.st) == Cerrados.end())
		{
			Cerrados.insert(current.st);

			if (mapaResultado[current.st.fila][current.st.columna] == 'K')
			{
				current.st.bikini = true;
				current.st.zapatillas = false;
			}
			else if (mapaResultado[current.st.fila][current.st.columna] == 'D')
			{
				current.st.bikini = false;
				current.st.zapatillas = true;
			}

			int k = 0;
			for (list<estado>::const_iterator it = objetivos.begin(); it != objetivos.end(); it++)
			{
				if (current.st.fila == it->fila && current.st.columna == it->columna)
				{
					current.st.objiConseguido[k] = true;
				}
				k++;
			}

			// Generar descendiente de girar a la derecha 90 grados
			nodo hijoTurnR = current;
			for (int i = 0; i < 3; i++)
			{
				hijoTurnR.st.objiConseguido[i] = current.st.objiConseguido[i];
			}
			hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
			if (Cerrados.find(hijoTurnR.st) == Cerrados.end())
			{
				hijoTurnR.coste = calcularCosteN3(current, actTURN_R, sensores);
				hijoTurnR.costeAcumulado = hijoTurnR.coste + current.costeAcumulado;
				hijoTurnR.heuristica = calcularHeuristicaN4(hijoTurnR, destinos);
				hijoTurnR.secuencia.push_back(actTURN_R);
				Abiertos.push(hijoTurnR);
			}

			// Generar descendiente de girar a la derecha 45 grados
			nodo hijoSEMITurnR = current;
			for (int i = 0; i < 3; i++)
			{
				hijoSEMITurnR.st.objiConseguido[i] = current.st.objiConseguido[i];
			}
			hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
			if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end())
			{
				hijoSEMITurnR.coste = calcularCosteN3(current, actSEMITURN_R, sensores);
				hijoSEMITurnR.costeAcumulado = hijoSEMITurnR.coste + current.costeAcumulado;
				hijoSEMITurnR.heuristica = calcularHeuristicaN4(current, destinos);
				hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
				Abiertos.push(hijoSEMITurnR);
			}

			// Generar descendiente de girar a la izquierda 90 grados
			nodo hijoTurnL = current;
			for (int i = 0; i < 3; i++)
			{
				hijoTurnL.st.objiConseguido[i] = current.st.objiConseguido[i];
			}
			hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;

			if (Cerrados.find(hijoTurnL.st) == Cerrados.end())
			{
				hijoTurnL.coste = calcularCosteN3(current, actTURN_L, sensores);
				hijoTurnL.costeAcumulado = hijoTurnL.coste + current.costeAcumulado;
				hijoTurnL.heuristica = calcularHeuristicaN4(current, destinos);
				hijoTurnL.secuencia.push_back(actTURN_L);
				Abiertos.push(hijoTurnL);
			}

			// Generar descendiente de girar a la izquierda 45 grados
			nodo hijoSEMITurnL = current;
			for (int i = 0; i < 3; i++)
			{
				hijoSEMITurnL.st.objiConseguido[i] = current.st.objiConseguido[i];
			}
			hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;

			if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end())
			{
				hijoSEMITurnL.coste = calcularCosteN3(current, actSEMITURN_L, sensores);
				hijoSEMITurnL.costeAcumulado = hijoSEMITurnL.coste + current.costeAcumulado;
				hijoSEMITurnL.heuristica = calcularHeuristicaN4(current, destinos);
				hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);
				Abiertos.push(hijoSEMITurnL);
			}

			// Generar descendiente de avanzar
			nodo hijoForward = current;

			if (!HayObstaculoDelante(hijoForward.st))
			{
				for (int i = 0; i < 3; i++)
				{
					hijoForward.st.objiConseguido[i] = current.st.objiConseguido[i];
				}

				if (Cerrados.find(hijoForward.st) == Cerrados.end())
				{
					hijoForward.coste = calcularCosteN3(current, actFORWARD, sensores);
					hijoForward.costeAcumulado = hijoForward.coste + current.costeAcumulado;
					hijoForward.heuristica = calcularHeuristicaN4(hijoForward, destinos);
					hijoForward.secuencia.push_back(actFORWARD);

					int i = 0;

					for (list<estado>::const_iterator it = objetivos.begin(); it != objetivos.end(); it++)
					{
						if (hijoForward.st.fila == it->fila && hijoForward.st.columna == it->columna)
						{
							hijoForward.st.objiConseguido[i] = true;
						}

						i++;
					}

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

	if ((current.st.objiConseguido[0] == true) and (current.st.objiConseguido[1] == true) and (current.st.objiConseguido[2] == true))
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

bool ComportamientoJugador::hayLoboCerca(Sensores sensores)
{

	for (int i = 0; i < 16; i++)
	{
		if (sensores.superficie[i] == 'l')
		{
			return true;
		}
	}
	return false;
}
void ComportamientoJugador::rellenarAproximacion()
{
	int contS = 0;
	int contP = 0;
	int contB = 0;
	int contA = 0;
	int contT = 0;
	int contM = 0;
	int max = 0;
	for (int i = 0; i < (mapaResultado.size()); i++)
	{
		for (int j = 1; j < (mapaResultado.size()); j++)
		{
		}
	}
	for (int i = 1; i < (mapaResultado.size() - 1); i++)
	{
		for (int j = 1; j < (mapaResultado.size() - 1); j++)
		{
			if (mapaResultado[i][j] == '?')
			{
				max = 2;
				contS = contP = contB = contA = contT = contM = 0;
				for (int k = j - 1; k <= j + 1; k++)
				{
					switch (mapaAproximacion[i - 1][k])
					{
					case 'S':
						contS++;
						break;
					case 'P':
						contP++;
						break;
					case 'B':
						contB++;
						break;
					case 'A':
						contA++;
						break;
					case 'T':
						contT++;
						break;
					case 'M':
						contM++;
						break;
					default:
						break;
					}
				}
				for (int k = j - 1; k <= j + 1; k++)
				{
					switch (mapaAproximacion[i][k])
					{
					case 'S':
						contS++;
						break;
					case 'P':
						contP++;
						break;
					case 'B':
						contB++;
						break;
					case 'A':
						contA++;
						break;
					case 'T':
						contT++;
						break;
					case 'M':
						contM++;
						break;
					default:
						break;
					}
				}
				for (int k = j - 1; k <= j + 1; k++)
				{
					switch (mapaResultado[i + 1][k])
					{
					case 'S':
						contS++;
						break;
					case 'P':
						contP++;
						break;
					case 'B':
						contB++;
						break;
					case 'A':
						contA++;
						break;
					case 'T':
						contT++;
						break;
					case 'M':
						contM++;
						break;
					default:
						break;
					}
				}

				if (contS > max)
				{
					mapaAproximacion[i][j] = 'S';
				}
				else if (contB > max)
				{
					mapaAproximacion[i][j] = 'B';
				}
				else if (contT > max)
				{
					mapaAproximacion[i][j] = 'T';
				}
				else if (contA > max)
				{
					mapaAproximacion[i][j] = 'A';
				}
				else if (contP > max)
				{
					mapaAproximacion[i][j] = 'P';
				}
				else if (contM > max)
				{
					mapaAproximacion[i][j] = 'M';
				}
			}
		}
	}

	for (int i = 1; i < (mapaResultado.size() - 1); i++)
	{
		for (int j = 1; j < (mapaResultado.size() - 1); j++)
		{
			if (mapaResultado[i][j] == '?')
			{
				max = 2;
				contS = contP = contB = contA = contT = contM = 0;
				for (int k = j - 1; k <= j + 1; k++)
				{
					switch (mapaResultado[i - 1][k])
					{
					case 'S':
						contS++;
						break;
					case 'P':
						contP++;
						break;
					case 'B':
						contB++;
						break;
					case 'A':
						contA++;
						break;
					case 'T':
						contT++;
						break;
					case 'M':
						contM++;
						break;
					default:
						break;
					}
				}
				for (int k = j - 1; k <= j + 1; k++)
				{
					switch (mapaResultado[i][k])
					{
					case 'S':
						contS++;
						break;
					case 'P':
						contP++;
						break;
					case 'B':
						contB++;
						break;
					case 'A':
						contA++;
						break;
					case 'T':
						contT++;
						break;
					case 'M':
						contM++;
						break;
					default:
						break;
					}
				}
				for (int k = j - 1; k <= j + 1; k++)
				{
					switch (mapaResultado[i + 1][k])
					{
					case 'S':
						contS++;
						break;
					case 'P':
						contP++;
						break;
					case 'B':
						contB++;
						break;
					case 'A':
						contA++;
						break;
					case 'T':
						contT++;
						break;
					case 'M':
						contM++;
						break;
					default:
						break;
					}
				}

				if (contS > max)
				{
					mapaResultado[i][j] = 'S';
				}
				else if (contB > max)
				{
					mapaResultado[i][j] = 'B';
				}
				else if (contT > max)
				{
					mapaResultado[i][j] = 'T';
				}
				else if (contA > max)
				{
					mapaResultado[i][j] = 'A';
				}
				else if (contP > max)
				{
					mapaResultado[i][j] = 'P';
				}
				else if (contM > max)
				{
					mapaResultado[i][j] = 'M';
				}
			}
		}
	}
}

int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}
