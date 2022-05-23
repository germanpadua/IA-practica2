#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <set>
#include <vector>

struct estado {
  int fila;
  int columna;
  int orientacion;

  bool zapatillas;
  bool bikini;

  bool objiConseguido[3];
};


struct nodo
{
	estado st;
	list<Action> secuencia;

  int coste;
	int costeAcumulado;
	int heuristica;

	inline bool operator<(const nodo & other) const{return (this->costeAcumulado + this->heuristica) > (other.costeAcumulado + other.heuristica);}
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      hayPlan = false;
      tamanio = size;
      batPosC = batPosF = -1;
      tieneBikini = tieneZapatillas = false;
      contador = 0;
      vaABateria = false;
      iteracion = 0;
      necesitaPos = true;
      posicionF = posicionC = -1;
      brujula = -1;
      ultimaAccion = actIDLE;
      ultObjF = ultObjC = -1;
      nObjConseguidos = 0;
      nObjTotales = 0;
      porcentajeDescubierto = 0;
      vaConTodo = false;
      noPuedeIr = 0;

      for(int i=0; i<3; i++)
        objiConseguido[i] = false;

      vector<unsigned int> aux(size, 0);
      vector<char> aux2(size, '?');
      

      for(int i=0; i < size; i++){
        mapaContador.push_back(aux);
        mapaAproximacion.push_back(aux2);
        mapaIntentos.push_back(aux);
      }
      
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      hayPlan = false;
      tamanio = mapaR.size();
      batPosC = batPosF = -1;
      tieneBikini = tieneZapatillas = false;
      contador = 0;
      vaABateria = false;
      iteracion = 0;
      necesitaPos = true;
      posicionF = posicionC = -1;
      brujula = -1;
      ultimaAccion = actIDLE;

      ultObjF = ultObjC = -1;
      nObjConseguidos = 0;
      nObjTotales = 0;
      porcentajeDescubierto = 0;

      vaConTodo = false;
      noPuedeIr =  0;


      for(int i=0; i<3; i++)
        objiConseguido[i] = false;

      vector<unsigned int> aux(tamanio, 0);
      vector<char> aux2(tamanio, '?');
      

      for(int i=0; i < tamanio; i++){
        mapaContador.push_back(aux);
        mapaAproximacion.push_back(aux2);
        mapaIntentos.push_back(aux);
      }
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:
    // Declarar Variables de Estado
    estado actual;
    list<estado> objetivos;
    list<Action> plan;
    
    bool hayPlan;
    int tamanio;
    bool poca_bateria;
    int batPosC;
    int batPosF;
    int contador;

    bool tieneBikini;
    bool tieneZapatillas;
    bool vaABateria;

    bool objiConseguido[3];
    int iteracion;
    bool necesitaPos;
    int posicionF;
    int posicionC;
    int brujula;
    int ultObjF;
    int ultObjC;
    int nObjConseguidos;
    int nObjTotales;
    float porcentajeDescubierto;
    bool vaConTodo;
    int noPuedeIr;
    Action ultimaAccion;

    vector<vector<unsigned int>> mapaContador;
    vector<vector<char>> mapaAproximacion;
    vector<vector<unsigned int>> mapaIntentos;

    set<pair<int, int>> baterias_vistas;

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const list<estado> &destino, list<Action> &plan, Sensores sensores);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_AEstrella(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_AEstrellaN3(const estado &origen, const estado &destino, list<Action> &plan, Sensores sensores);
    bool pathFinding_AEstrellaN4(const estado &origen, const list<estado> &destino, list<Action> &plan, Sensores sensores);

    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);

    int calcularCosteN3(nodo & nodo, const Action & accion, Sensores sensores);
    int calcularCoste(nodo & nodo, const Action & accion);

    int calcularHeuristica(nodo & n, const estado & objetivo);
    int calcularHeuristicaN4(nodo & n, const list<estado> &objetivos);
    
    void rellenarMapa(Sensores sensores);
    void rellenarAproximacion();
    
    int calcularDistancia(int x1, int y1, int x2, int y2);
    bool hayLoboCerca(Sensores sensores);
    float porcentajeCerca(Sensores sensores);

    pair<int, int> buscarObjetivo(Sensores sensores);
};

#endif
