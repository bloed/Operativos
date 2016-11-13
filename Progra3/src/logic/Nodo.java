package logic;

import java.util.ArrayList;

public class Nodo {

  private String tipo; //archivo o directorio, o root
  private String nombre;
  private String extension;
  private String contenido;
  private Integer tamano;
  private ArrayList<Nodo> hijos;
  private ArrayList<Sector> sectores;//contiene los sectores tambien. de 1 a N.
  private Nodo padre;
  
  public Nodo(String pTipo, String pNombre,String pExtension, String pContenido,Nodo pPadre) {
    tipo = pTipo;
    nombre = pNombre;
    extension = pExtension;
    hijos = new ArrayList<Nodo>();
    sectores = new ArrayList<Sector>();
    padre = pPadre;
    contenido = pContenido;
    tamano = contenido.length();
  }
  
  public void setPadre(Nodo pPadre){
    padre = pPadre;
  }
  public Nodo getPadre(){
    return padre;
  }
  public String getTipo(){
    return tipo;
  }
  
  public String getNombre(){
    return nombre;
  }
  public Integer getTamano(){
    return tamano;
  }
  public String getContenido(){
    return contenido;
  }
  
  public String getPath(){
    return getPathAux("", this);
  }
  private String getPathAux(String resultado, Nodo actual){
    if (actual == null){
      resultado = "C:/" + resultado;
      return resultado;
    }
    else{
      resultado = actual.getNombre() + "/" + resultado ;
      return getPathAux(resultado, actual.getPadre());
    }
    
  }
  
  public void agregarHijo(Nodo pNodo){
    hijos.add(pNodo);
    pNodo.setPadre(this);
  }
  
  public String toString(){
    return tipo + " | " + nombre + " | " + extension;
  }
}
