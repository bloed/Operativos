package logic;

import java.util.ArrayList;

public class Nodo {

  private String tipo; //archivo o directorio, o root
  private String nombre;
  private ArrayList<Nodo> hijos;
  private ArrayList<Sector> sectores;//contiene los sectores tambien. de 1 a N.
  
  public Nodo(String pTipo, String pNombre) {
    tipo = pTipo;
    nombre = pNombre;
    hijos = new ArrayList<Nodo>();
    sectores = new ArrayList<Sector>();
  }
  
  public String getTipo(){
    return tipo;
  }
  
  public String getNombre(){
    return nombre;
  }
  
  public String toString(){
    return tipo + " | " + nombre;
  }
}
