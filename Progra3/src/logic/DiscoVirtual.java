package logic;

import java.io.PrintWriter;
import java.util.ArrayList;

public class DiscoVirtual {
  
  private Integer cantidadSectores;
  private Integer tamanoSectores;
  private Integer tamanoTotal;
  private Nodo root;
  private ArrayList<Sector> sectores;

  public DiscoVirtual(Integer pCantidadSectores, Integer pTamanoSectores) {
    cantidadSectores = pCantidadSectores;
    tamanoSectores = pTamanoSectores;
    tamanoTotal = pCantidadSectores * pTamanoSectores;
    root = new Nodo("root","root","","",null);
    sectores = new ArrayList<Sector>();
    inicializarSectores();
  }
  
  public Nodo getRoot(){
    return root;
  }
  
  public Integer getTamanoSectores(){
    return tamanoSectores;
  }
  
  private void inicializarSectores(){
    for(int i = 0; i < cantidadSectores; i++){
      sectores.add(new Sector(tamanoSectores,"Vacio"));
    }
  }
  
  public String toString() {
    return "Cantidad Sectores: " + cantidadSectores + ". Tamaño de cada sector: " + tamanoSectores + 
        ". Tamaño total: " +  tamanoTotal + ".";
  }
  
  
  public String asignarArchivo(Nodo pArchivo){
    //asigna a los sectores disponibles el arhivo
    String contenido= pArchivo.getContenido();
    Integer tamano = pArchivo.getTamano();
    Integer sectoresNecesarios = (tamano + tamanoSectores - 1) / tamanoSectores;
    if (haySectores(sectoresNecesarios)){
      for(Sector s : sectores){
        //ahora sí lo asignamos
        if(!s.estaUsado() && contenido.length()!= 0){
            //conseguimos los primeros bytes
            s.setArchivo(pArchivo,contenido.substring(0, Math.min(contenido.length(), tamanoSectores)));
            contenido = contenido.substring(Math.min(contenido.length(), tamanoSectores),contenido.length());
            pArchivo.agregarSector(s);
        }
      }
      return "Archivo Creado.";
    }
    else{
      return "No hay espacio suficiente en disco.";
    }
  }
  public Boolean haySectores(int cantidad){
    //devuelve si hay tal cantida de sectores libres en el disco
    if (cantidad <= 0){
      return true;
    }
    int cantidadEncontrada = 0;
    for(Sector s : sectores){
      if(!s.estaUsado()){
        cantidadEncontrada++;
      }
    }
    return cantidadEncontrada >= cantidad;
  }
  
  public void toDiscoFisico(){
    //debe pasar Todo lo que esta en el logico al disco.
    try{
      PrintWriter writer = new PrintWriter("Disco.txt", "UTF-8");
      for(Sector s: sectores){
        //resultado += s.getContenido() + "\n";
        writer.println(s.getContenido());
      }
      writer.close();
    } 
    catch (Exception e) {
       System.out.println(e.getMessage());
    }
  }

}
