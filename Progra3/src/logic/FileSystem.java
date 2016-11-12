package logic;

public class FileSystem {

  //contiene todas las funciones
  
  public FileSystem() {
    // TODO Auto-generated constructor stub
  }
  
  public static String action(String action,Nodo pNodo, String nombre){
    //switch cochino con todas las acciones
    switch (action){
    case "file":
      return "Se ha creado un archivo.";
    case "mkdir":
      return "Se ha creado un directorio.";
    default:
      return "Se ha ingresado una accion inexistente.";
    }
  }

}
