package logic;

public class FileSystem {

  
  private DiscoVirtual disco;
  private Nodo actual;// nodo actual en el que esta el padre
  Integer tamanoSectores;
  //contiene todas las funciones
  
  public FileSystem(DiscoVirtual pDisco) {
    disco = pDisco;
    actual = disco.getRoot();
    tamanoSectores = disco.getTamanoSectores();
  }
  
  public Nodo getActual(){
    return actual;
  }
  public void setActual(Nodo pNodo){
    actual = pNodo;
  }
  
  public String action(String action){
    //switch cochino con todas las acciones
    String nombre = "";
    String extension = "";
    Nodo archivo;
    String path = "";
    String resultado = "";
    switch (action){
    case "file":
      System.out.println("Ingresa el nombre del archivo:");
      nombre = Main.getInputString();
      System.out.println("Ingresa la extensión del archivo:");
      extension = Main.getInputString();
      System.out.println("Ingresa el contenido del archivo:");
      String contenido = Main.getInputString();
      System.out.println("Ingresa el path del archivo. No poner nada si se quiere en el directorio actual:");
      path = Main.getInputString();
      if(!path.equals("")){
        resultado = cambiarDirectorio(path);
      }
      if (resultado.equals("Path incorrecto. No se encontró un directorio a ese path.")){
        return resultado;
      }
      else{
        return crearArchivo(nombre, extension, contenido);
      }
      
    case "mkdir":
      System.out.println("Ingresa el nombre del directorio:");
      nombre = Main.getInputString();
      return crearDirectorio(nombre);
      
    case "cambiardir":
      System.out.println("Ingresa al directorio que se quiere ir. Ej.: c://root/directorio o directorio o ../directorio :");
      nombre = Main.getInputString();
      return cambiarDirectorio(nombre);
      
    case "listadir":
      return actual.listaDir("  ");
      
    case "tree":
      tree();
      return "";
      
    case "verpropiedades":
      System.out.println("Ingresa el nombre del archivo:");
      nombre = Main.getInputString();
      System.out.println("Ingresa la extensión del archivo:");
      extension = Main.getInputString();
      System.out.println("Ingresa el path del archivo. No poner nada si se quiere en el directorio actual:");
      path = Main.getInputString();
      if(!path.equals("")){
        resultado = cambiarDirectorio(path);
      }
      if (resultado.equals("Path incorrecto. No se encontró un directorio a ese path.")){
        return resultado;
      }
      else{
        archivo = actual.tieneArchivo(nombre, extension);
        if(archivo == null){
          return "No se encontro un archivo con esa extensión.";
        }
        else{
          return archivo.verPropiedades();
        }
      }
      
    case "modfile":
      System.out.println("Ingresa el nombre del archivo:");
      nombre = Main.getInputString();
      System.out.println("Ingresa la extensión del archivo:");
      extension = Main.getInputString();
      System.out.println("Ingresa el nuevo contenido del archivo (será sobreescrito):");
      contenido = Main.getInputString();
      System.out.println("Ingresa el path del archivo. No poner nada si se quiere en el directorio actual:");
      path = Main.getInputString();
      if(!path.equals("")){
        resultado = cambiarDirectorio(path);
      }
      if (resultado.equals("Path incorrecto. No se encontró un directorio a ese path.")){
        return resultado;
      }
      else{
        archivo = actual.tieneArchivo(nombre, extension);
        if(archivo == null){
          return "No se encontro un archivo con esa extensión.";
        }
        else{
          Integer sectoresActuales = (archivo.getContenido().length() + tamanoSectores - 1) / tamanoSectores;
          Integer sectoresAdicionales = (contenido.length() + tamanoSectores - 1) / tamanoSectores;
          System.out.println("Sectores actuales : " + sectoresActuales);
          System.out.println("Sectores adicionales : " + sectoresAdicionales);
          if(disco.haySectores(sectoresAdicionales - sectoresActuales)){
            //primero vemos si hay espacio para el nuevo contenido
            //parte de la idea de que lo actual ya esta "reservado" para el archivo, puedee que necesite menos, más, o igual
            archivo.quitaSectores();
            archivo.setContenido(contenido);
            return disco.asignarArchivo(archivo);//aqui sabemos que de fijo ya hay espacio
          }
          else{
            return "No hay suficiente espacio en disco para el nuevo contenido.";
          }
        }
      }
      
    case "contfile":
      System.out.println("Ingresa el nombre del archivo:");
      nombre = Main.getInputString();
      System.out.println("Ingresa la extensión del archivo:");
      extension = Main.getInputString();
      System.out.println("Ingresa el path del archivo. No poner nada si se quiere en el directorio actual:");
      path = Main.getInputString();
      if(!path.equals("")){
        resultado = cambiarDirectorio(path);
      }
      if (resultado.equals("Path incorrecto. No se encontró un directorio a ese path.")){
        return resultado;
      }
      else{
        archivo = actual.tieneArchivo(nombre, extension);
        if(archivo == null){
          return "No se encontro un archivo con esa extensión.";
        }
        else{
          return archivo.getContenido();
        }
      }
      
    default:
      return "Se ha ingresado una accion inexistente.";
    }
  }
  
  public String crearArchivo(String pNombre, String pExtension, String pContenido){
    if(actual.tieneArchivo(pNombre, pExtension) == null){
      Nodo archivo = new Nodo("archivo", pNombre, pExtension, pContenido, actual);
      String resultado = disco.asignarArchivo(archivo);
      if (resultado.equals("Archivo Creado.")){
        actual.agregarHijo(archivo);
        //actual = archivo;
      }
      return resultado;
    }
    else{
      return "Ya existe un archivo con ese nombre y extensión.";
    }
  }
  
  public String crearDirectorio(String pNombre){
    if(actual.tieneDirectorio(pNombre) == null){
      Nodo directorio = new Nodo("dir", pNombre, "", "", actual);
      actual.agregarHijo(directorio);
      //actual = directorio;
      return "Directorio creado";
    }
    else{
      return "Ya existe un directorio con ese nombre.";
    }
  }
  public String cambiarDirectorio(String path){
    Nodo resultado = actual.cambiarPath(path, disco.getRoot());
    if (resultado == null){
      return "Path incorrecto. No se encontró un directorio a ese path.";
    }
    else{
      actual = resultado;
    }
    return "";
  }
  
  public String tree(){
    treeAUX("", disco.getRoot());    
    return "";
  }
  private void treeAUX(String nivel, Nodo actual){
    if (actual == null){//obvio los archivos no aplica
      return;
    }
    System.out.println(nivel + actual.getNombre() + "." + actual.getExtension());
    if (actual.getTipo().equals("archivo")){//obvio los archivos no aplica
      return;
    }
    for(Nodo hijo : actual.getHijos()){
      //se debe llamar recursivamente para cada hijo
      treeAUX(nivel + "  ", hijo);
    }
  }

}
