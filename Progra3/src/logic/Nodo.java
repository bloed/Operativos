package logic;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;

public class Nodo {

  private String tipo; //archivo o directorio, o root
  private String nombre;
  private String extension;
  private String contenido;
  private Integer tamano;
  private ArrayList<Nodo> hijos;
  private ArrayList<Sector> sectores;//contiene los sectores tambien. de 1 a N.
  private Nodo padre;
  String fechaCreacion;
  String fechaModificacion;
  
  public Nodo(String pTipo, String pNombre,String pExtension, String pContenido,Nodo pPadre) {
    tipo = pTipo;
    nombre = pNombre;
    extension = pExtension;
    hijos = new ArrayList<Nodo>();
    sectores = new ArrayList<Sector>();
    padre = pPadre;
    contenido = pContenido;
    tamano = contenido.length();
    fechaCreacion = getFechaActual();
    fechaModificacion = getFechaActual();
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
  public String getExtension(){
    return extension;
  }
  public String getContenido(){
    return contenido;
  }
  public ArrayList<Sector> getSectores(){
    return sectores;
  }
  public void agregarSector(Sector pSector){
    sectores.add(pSector);
  }
  
  public String getPath(){
    return getPathAux("", this);
  }
  private String getPathAux(String resultado, Nodo actual){
    if (actual == null){
      resultado = "c://" + resultado;
      return resultado;
    }
    else{
      resultado = actual.getNombre() + "/" + resultado ;
      return getPathAux(resultado, actual.getPadre());
    }
    
  }
  
  public Nodo tieneDirectorio(String pNombre){
    //revisa si tiene un directorio con el nombre dado
    for(Nodo hijo: hijos){
      if (hijo.getNombre().equals(pNombre) && hijo.getTipo().equals("dir")){
        return hijo;
      }
    }
    return null;
  }
  
  public Nodo tieneArchivo(String pNombre, String pExtension){
    //revisa si existe ese archivo con esa extensión
    for(Nodo hijo: hijos){
      if (hijo.getNombre().equals(pNombre) && hijo.getTipo().equals("archivo")
          && hijo.getExtension().equals(pExtension)){
        return hijo;
      }
    }
    return null;
  }
  
  public Nodo cambiarPath(String path, Nodo root){
    //recibe un path de cualquier forma, y devuelve un nodo hacia ese path.
    //si devuelve nulo es que no encontro
    //recibe el root para que quede más cómodo a la hora de rutas absolutas
    String s = path.substring(0, Math.min(path.length(), 9));
    //System.out.println(s);
    if(s.equals("c://root")){
      return root;
    }
    if(s.equals("c://root/")){
      //rutas absolutas
      path = path.substring(Math.min(path.length(), 9), path.length());
      return cambiarPathAUX(path, root);
    }
    else{
      //rutas relativas
      return cambiarPathAUX(path, this);
    }
    
  }
  
  public Nodo cambiarPathAUX(String path, Nodo actual){
    //System.out.println(path);
    if(actual == null){
      return null;
    }
    else if(path.length() == 0){
      return actual;
    }
    else if(path.substring(0, Math.min(path.length(), 3)).equals("../")){
      return cambiarPathAUX(path.substring(Math.min(path.length(), 3)), actual.getPadre());
    }
    else if(path.substring(0, Math.min(path.length(), 2)).equals("..")){
      return actual.getPadre();
    }
    else{//es el nombre de un directorio
      int index = path.indexOf("/");
      if (index < 0){
        index = 666;
      }
      String nombreDirectorio = path.substring(0, Math.min(path.length(), index));
      Nodo hijo = actual.tieneDirectorio(nombreDirectorio);
      if (hijo == null){
        return null;
      }
      else{
        return cambiarPathAUX(path.substring(Math.min(path.length(), index+1), path.length())
            ,hijo);
      }
    }
  }
  
  public String listaDir(String extra){
    //imprime todos los hijos de un directorio
    //el extra es para darle un indent
    String resultado = "";
    for(Nodo hijo: hijos){
      resultado += extra + hijo.toString() + "\n";
    }
    return resultado;
  }
  
  public ArrayList<Nodo> getHijos(){
    return hijos;
  }
  
  public void agregarHijo(Nodo pNodo){
    hijos.add(pNodo);
    pNodo.setPadre(this);
  }
  
  public String getFechaActual(){
    Calendar cal = Calendar.getInstance();
    DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
    return dateFormat.format(cal.getTime());
  }
  
  public String toString(){
    return tipo + " | " + nombre + "." + extension;
  }
  
  public String verPropiedades(){
    String resultado = "";//aplica solo para archivos
    if (tipo.equals("archivo")){
      resultado += nombre + "." + extension + "\n";
      resultado += "Fecha de creación : " + fechaCreacion + ". Fecha modificación: " + fechaModificacion;
      resultado += ".\n" + "Tamaño: " + contenido.length() + " bytes.";
    }
    return resultado;
  }
}
