package logic;

public class Sector {
  
  //representa un sector logico. lo ideal es que lo que esta aca sea exactamente igual al o que esta en disco.

  private Integer id;
  private Integer tamano;
  private String contenido;
  private Nodo archivo;//contiene el nodo arhivo
  
  public static int contador = 0;
  
  public Sector(int pTamano) {
    contador++;
    id = contador;
    contenido = "";
  }
  
  public void setArchivo(Nodo pArchivo){
    archivo = pArchivo;
  }

}
