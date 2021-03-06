package logic;

import java.io.Serializable;

public class Sector implements Serializable {
  
  //representa un sector logico. lo ideal es que lo que esta aca sea exactamente igual al o que esta en disco.

  private Integer id;
  private Integer tamano;
  private String contenido;
  private Nodo archivo;//contiene el nodo arhivo
  private Boolean usado;
  
  public static int contador = 0;
  
  public Sector(int pTamano, String pContenido) {
    id = contador;
    contador++;
    contenido = pContenido;
    usado = false;
    archivo = null;
  }
  
  public void setContenido(String pContenido){
    contenido = pContenido;
  }
  
  public void setArchivo(Nodo pArchivo){
    archivo = pArchivo;
  }
  
  public String getContenido(){
    return contenido;
  }
  
  public void setArchivo(Nodo pArchivo, String informacion){
    archivo = pArchivo;
    contenido = informacion.replace("\n", "").replace("\r", "");
    usado = true;
  }
  
  public void setUsado(Boolean pUsado){
    usado = pUsado;
  }
  public Boolean estaUsado(){
    return usado;
  }

}
