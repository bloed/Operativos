package logic;

public class FileSystem {

  
  private DiscoVirtual disco;
  private Nodo actual;// nodo actual en el que esta el padre
  //contiene todas las funciones
  
  public FileSystem(DiscoVirtual pDisco) {
    disco = pDisco;
    actual = disco.getRoot();
  }
  
  public Nodo getActual(){
    return actual;
  }
  public void setActual(Nodo pNodo){
    actual = pNodo;
  }
  
  public String action(String action){
    //switch cochino con todas las acciones
    switch (action){
    case "file":
      System.out.println("Ingresa el nombre del archivo:");
      String nombre = Main.getInputString();
      System.out.println("Ingresa la extensión del archivo:");
      String extension = Main.getInputString();
      System.out.println("Ingresa el contenido del archivo:");
      String contenido = Main.getInputString();
      return crearArchivo(nombre, extension, contenido);
    case "mkdir":
      return "Se ha creado un directorio.";
    default:
      return "Se ha ingresado una accion inexistente.";
    }
  }
  
  public String crearArchivo(String pNombre, String pExtension, String pContenido){
    Nodo archivo = new Nodo("archivo", pNombre, pExtension, pContenido, actual);
    String resultado = disco.asignarArchivo(archivo);
    if (resultado.equals("Archivo Creado.")){
      actual.agregarHijo(archivo);
      actual = archivo;
    }
    disco.toDiscoFisico();
    return resultado;
  }


}
