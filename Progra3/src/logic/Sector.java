package logic;

public class Sector {
  
  //representa un sector logico. lo ideal es que lo que esta aca sea exactamente igual al o que esta en disco.

  private Integer id;
  private Integer tamano;
  private String contenido;
  private Nodo archivo;//contiene el nodo arhivo
  private Boolean usado;
  
  public static int contador = 0;
  
  public Sector(int pTamano) {
    id = contador;
    contador++;
    contenido = "";
    usado = false;
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
  public Boolean estaUsado(){
    return usado;
  }

}
