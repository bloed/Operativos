package logic;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintWriter;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

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
    Nodo directorio;
    String path = "";
    String resultado = "";
    String opcion = "n";
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
      
    case "listardir":
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
            archivo.setFechaModificacion();//actualizamos la fecha de modificacion
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
    
    case "remove":
      Boolean control = true;
      while(control){
        System.out.println("Ingresa el nombre del archivo o directorio (debe estar en el directorio actual):");
        nombre = Main.getInputString();
        System.out.println("Ingresa la extensión del archivo (ignorar si es un directorio):");
        extension = Main.getInputString();
        if (extension.equals("")){//es un directorio
          directorio = actual.tieneDirectorio(nombre);
          if(directorio == null){
            System.out.println("No se encontro un directorio directorio actual.");
          }
          else{
            //hay que ver si se quiere recursivo o no.
            if(directorio.tieneHijos()){
              System.out.println("El directorio tiene hijos, ¿Desea borrar recursivamente?  (y/n):");
              opcion = Main.getInputString();
              if(opcion.equals("y")){
                directorio.quitaSectoresRecursivo();
                actual.removerHijo(directorio);
                System.out.println("Directorio borrado junto con sus hijos.");
              }
              else{
                System.out.println("Directorio no fue borrado.");
              }
            }
            else{
              actual.removerHijo(directorio);
              System.out.println("Directorio borrado. No tenía hijos entonces se podía borrar directamente.");
            }
          }
        }
        else{ 
          archivo = actual.tieneArchivo(nombre, extension);
          if(archivo == null){
            System.out.println("No se encontro un archivo con esa extensión en el directorio actual.");
          }
          else{
            archivo.quitaSectores();//le quitamos los sectores en el disco
            actual.removerHijo(archivo);//quitamos el hijo del padre
            System.out.println("Archivo borrado correctamente.");
          }
        }
        System.out.println("¿Desea seguir borrando archivos/directorios?  (y/n):");
        opcion = Main.getInputString();
        if (opcion.equals("n")){
          control = false;
        }
      }
      return "";
      
    case "mover":
      Nodo directorioOrigen = null;
      Nodo directorioDestino = null;
      Nodo archivoOrigen = null;
      System.out.println("Ingresa el path de origen a un directorio (ya sea relativo o absoluto): (ignorar si es para el path actual)");
      String pathOrigen = Main.getInputString();
      if(pathOrigen.equals("")){
        directorioOrigen = actual;
      }
      else{
        directorioOrigen = actual.cambiarPath(pathOrigen, disco.getRoot());
      }
      
      System.out.println("Ingresa el nombre del archivo o directorio origen:");
      nombre = Main.getInputString();
      System.out.println("Ingresa la extensión del archivo origen (ignorar si es un directorio):");
      extension = Main.getInputString();
      
      
      System.out.println("Ingresa el path de destino a un directorio (ya sea relativo o absoluto): (ignorar si es para el path actual)");
      String pathDestino = Main.getInputString();
      if(pathDestino.equals("")){
        directorioDestino = actual;
      }
      else{
        directorioDestino = actual.cambiarPath(pathDestino, disco.getRoot());
      } 
      
      System.out.println("Ingresa el nombre del archivo o directorio para el path destino:");
      String nombreDestino = Main.getInputString();
      System.out.println("Ingresa la extensión del archivo destino (ignorar si es un directorio):");
      String extensionDestino = Main.getInputString();
      
      if (directorioOrigen == null){
        return "Directorio de origen inexistente";
      }
      if (directorioDestino == null){
        return "Directorio de destino inexistente";
      }
      
      if (extension.equals("")){//es un directorio
        archivoOrigen = directorioOrigen.tieneDirectorio(nombre);
      }
      else{
        archivoOrigen = directorioOrigen.tieneArchivo(nombre, extension);
      }
      
      if (archivoOrigen == null){
        return "Archivo en el directorio de origen, inexistente";
      }
      
      if (extensionDestino.equals("")){//es un directorio
        if(directorioDestino.tieneDirectorio(nombreDestino) != null){
          return "Ya existe un directorio con ese nombre en el destino";
        }
      }
      else{
        if(directorioDestino.tieneArchivo(nombreDestino, extensionDestino) != null){
          return "Ya existe un archivo con ese nombre y extension en el destino";
        }
      }
      
      archivoOrigen.setNombre(nombreDestino);
      archivoOrigen.setExtension(extensionDestino);
      directorioDestino.agregarHijo(archivoOrigen);
      directorioOrigen.removerHijo(archivoOrigen);
      return "Movida realizada!";

    case "copy":
        Nodo directorioOrigenCopy = null;
        Nodo directorioDestinoCopy = null;
        Nodo archivoOrigenCopy = null;
        Boolean origenReal = false;
        Boolean destinoReal = false;
        
        System.out.println("Ingresa el path de origen a un directorio (ya sea relativo o absoluto): (ignorar si es para el path actual)");
        String pathOrigenCopy = Main.getInputString();
        if(pathOrigenCopy.equals("")){
          directorioOrigenCopy = actual;
        }
        else{
          directorioOrigenCopy = actual.cambiarPath(pathOrigenCopy, disco.getRoot());
          if (directorioOrigenCopy == null){
              if(!validarReal(pathOrigenCopy)){
            	  return "Directorio o archvio no encontrado";
              }else{
            	  pathOrigenCopy += "\\";
            	  origenReal = true;
              }
          }
        }
        
        System.out.println("Ingresa el nombre del archivo o directorio origen:");
        nombre = Main.getInputString();
        System.out.println("Ingresa la extensión del archivo origen (ignorar si es un directorio):");
        extension = Main.getInputString();
        if(origenReal){
        	pathOrigenCopy += nombre;
        	if(!extension.isEmpty())
        		pathOrigenCopy += "."+ extension;
        }
        
        
        System.out.println("Ingresa el path de destino a un directorio (ya sea relativo o absoluto): (ignorar si es para el path actual)");
        String pathDestinoCopy = Main.getInputString();
        if(pathDestinoCopy.equals("")){
          directorioDestinoCopy = actual;
        }
        else{
          directorioDestinoCopy = actual.cambiarPath(pathDestinoCopy, disco.getRoot());
          if (directorioDestinoCopy == null){
              if(!validarReal(pathDestinoCopy)){
            	  return "Directorio o archvio no encontrado";
              }else{
            	  pathDestinoCopy += "\\";
            	  destinoReal = true;
              }
          }
        } 
        
        if(origenReal){
        	resultado = cambiarDirectorio(pathDestinoCopy);
        	String result = copyRtoV(pathOrigenCopy);
        	if(result.equals("No hay espacio suficiente en disco.")){
        		result = "No se pudieron copiar todos los archivos por espacio en el disco";
        	}
        	return result;
        	
        }
        else if(destinoReal){
        	Nodo inicial = actual.tieneArchivo(nombre, extension);
        	Boolean archivoCopy = true;
        	if(inicial == null){
        		inicial = actual.tieneDir(nombre);
        		archivoCopy = false;
        	}
        	if(inicial == null){
        		return "Archivo o directorio de origen no encontrado";
        	}
        	return copyVtoR(pathDestinoCopy, inicial, archivoCopy);
        }else{
        	Nodo inicial = actual.tieneArchivo(nombre, extension);
        	Boolean archivoCopy = true;
        	if(inicial == null){
        		inicial = actual.tieneDir(nombre);
        		archivoCopy = false;
        	}
        	if(inicial == null){
        		return "Archivo o directorio de origen no encontrado";
        	}
        	String result = copyVtoV(inicial,directorioDestinoCopy, archivoCopy);
        	if(result.equals("No hay espacio suficiente en disco.")){
        		result = "No se pudieron copiar todos los archivos por espacio en el disco";
        	}
        	return result;
        }
    case "find":
    	System.out.println("Ingrese el termino a buscar, puede utilizar '*.doc' para extensiones");
        String query = Main.getInputString();
        Boolean extensionSearch = false;
  	  	if(query.startsWith("*")){
  	  		query = query.substring(1);
  	  		extensionSearch = true;
  	  	}
  	  	return "Se obtuvieron :" + buscar(query, extensionSearch, disco.getRoot()) + " Resultado(s)";
  	  	
  	  	
    default:
      return "Se ha ingresado una accion inexistente.";
    }
  }
  
  private int buscar(String query, Boolean extension, Nodo actual){
	  int result = 0;
      String completo = actual.getNombre();
      if(actual.esArchivo())
    	  completo +="."+ actual.getExtension();
	  if(extension){
		  if(completo.endsWith(query)){
			  String path = actual.getPath();
			  System.out.println(path.substring(0, path.length()-1)+"."+actual.getExtension());
			  result ++;
		  }
	  }else{
		  if(completo.contains(query)){
			  String path = actual.getPath();
			  System.out.println(path.substring(0, path.length()-1)+"."+actual.getExtension());
			  result ++;
		  }
	  }
	  for(Nodo hijo: actual.getHijos()){
		  result += buscar(query,extension,hijo);
	  }
      return result;
  }
  
  public String crearArchivo(String pNombre, String pExtension, String pContenido){
    String opcion = "";
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
      //return "Ya existe un archivo con ese nombre y extensión.";
      System.out.println("Ya existe un archivo con ese nombre y extensión, ¿desea reemplazarlo? (y/n)");
      opcion = Main.getInputString();
      if (opcion.equals("y")){
        Nodo archivoExistente = actual.tieneArchivo(pNombre, pExtension);

        Integer sectoresActuales = (archivoExistente.getContenido().length() + tamanoSectores - 1) / tamanoSectores;
        Integer sectoresAdicionales = (pContenido.length() + tamanoSectores - 1) / tamanoSectores;
        System.out.println("Sectores actuales : " + sectoresActuales);
        System.out.println("Sectores adicionales : " + sectoresAdicionales);
        
        if(disco.haySectores(sectoresAdicionales - sectoresActuales)){
          //primero vemos si hay espacio para el nuevo archivo; tomando en cuenta lo que tenemos ahora
          //parte de la idea de que lo actual ya esta "reservado" para el archivo, puedee que necesite menos, más, o igual
          archivoExistente.quitaSectores();
          actual.removerHijo(archivoExistente);//quitamos el hijo del padre
          Nodo nuevoArchivo = new Nodo("archivo", pNombre, pExtension, pContenido, actual);
          actual.agregarHijo(nuevoArchivo);
          return disco.asignarArchivo(nuevoArchivo);//aqui sabemos que de fijo ya hay espacio
        }
        else{
          return "No hay suficiente tamaño en disco para el nuevo archivo. No se reemplaza el existente";
        }
      }
      else{
        return "No se reemplaza el archivo existente.";
      }
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
      Nodo directorio = actual.tieneDirectorio(pNombre);
      System.out.println("Ya existe un archivo con ese nombre y extensión, ¿desea reemplazarlo (también se borrará los hijos si lo posee)? (y/n)");
      String opcion = Main.getInputString();
      if (opcion.equals("y")){
        directorio.quitaSectoresRecursivo();
        actual.removerHijo(directorio);
        
        Nodo directorioNuevo = new Nodo("dir", pNombre, "", "", actual);
        actual.agregarHijo(directorioNuevo);
        return "Nuevo directorio creado.";
      }
      else{
        return "No se reemplaza el directorio";
      }
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
  private String copyVtoR(String folderDestino, Nodo actualCopy, Boolean archivo){
	  if(archivo){
		  folderDestino += actualCopy.getNombre() + "." + actualCopy.getExtension();
		  crearArchivoReal(folderDestino, actualCopy.getContenido());
	  }else{
		  crearDirectorioReal(folderDestino + actualCopy.getNombre());
		  for(Nodo hijo : actualCopy.getHijos()){
			  copyVtoR(folderDestino + actualCopy.getNombre() + "\\", hijo, hijo.esArchivo());
		  }
	  }
	  return "Copy de Virutal a Real Listo";
  }
  private String copyVtoV(Nodo origen, Nodo destino, Boolean archivo){
	  if(archivo){
		  Nodo act = this.actual;
		  this.actual = destino;
		  return crearArchivo(origen.getNombre(), origen.getExtension(), origen.getContenido());
	  }else{
		  Nodo temp = this.actual;
		  this.actual = destino;
		  crearDirectorio(origen.getNombre());
		  cambiarDirectorio(origen.getNombre());
		  for(Nodo hijo : origen.getHijos()){
			  String result = copyVtoV(hijo,this.actual, hijo.esArchivo());
			  if(result.equals("No hay espacio suficiente en disco.")){
				  this.actual = temp;
				  return "No hay espacio suficiente en disco.";
			  }
		  }
		  this.actual = temp;
	  }
	  
	  return "Copy de Virutal a Virtual Listo";
  }
  private String copyRtoV(String folder){
	  File dirFile = new File(folder);
	  File[] contents = dirFile.listFiles();
	  String fileName;
	  String extension = "";
	  String content;
	  
	  if(dirFile.isDirectory()){
		  System.out.println(dirFile.getName());
		  crearDirectorio(dirFile.getName());
		  cambiarDirectorio(dirFile.getName());
		  for ( File f : contents) {
			extension = "";
			fileName = f.getName();
			int i = fileName.lastIndexOf('.');
			if (i >= 0) 
			    extension = f.getName().substring(i+1);
			
			if(f.isDirectory()){
				Nodo temp = this.actual;
				copyRtoV(f.getAbsolutePath());
				this.actual = temp;
				
			}else{ //es archivo
				content = getAllContent(f.getAbsolutePath());
				fileName = fileName.replaceFirst("[.][^.]+$", "");
				String result = crearArchivo(fileName,extension, content);
				if(result.equals("No hay espacio suficiente en disco.")){
					return "No hay espacio suficiente en disco.";
				}
			}
		  }
	  }else{
		  fileName = dirFile.getName();
		  int i = fileName.lastIndexOf('.');
		  if (i >= 0)
			  extension = fileName.substring(i+1);
		  
		  content = getAllContent(dirFile.getAbsolutePath());
		  fileName = fileName.replaceFirst("[.][^.]+$", "");
		  String result = crearArchivo(fileName, extension, content);
		  if(result.equals("No hay espacio suficiente en disco.")){
			  return "No hay espacio suficiente en disco.";
		  }
	  }
	  return "Copy de Real Virtual listo";
  }
  private String getAllContent(String absolutePath){
	  String content = "";
	  try {
		  content = new String(Files.readAllBytes(Paths.get(absolutePath)));
	  } catch (IOException e) {
		  System.out.println("Error de path o permisos Real");
	  }
	  return content;
  }
  private Boolean validarReal(String absolutePath){
	  File file = new File(absolutePath);
	  if(file.exists())
		  return true;
	  else
		  return false;
  }
  private void crearArchivoReal(String path, String content){
	  try{
		    PrintWriter writer = new PrintWriter(path, "UTF-8");
		    writer.println(content);
		    writer.close();
		} catch (Exception e) {
		   System.out.println("Error de escritura real");
		}
  }
  private void crearDirectorioReal(String path){
	  new File(path).mkdir();
  }
  private Object copy(Object orig) {
      Object obj = null;
      try {
          // Write the object out to a byte array
          ByteArrayOutputStream bos = new ByteArrayOutputStream();
          ObjectOutputStream out = new ObjectOutputStream(bos);
          out.writeObject(orig);
          out.flush();
          out.close();

          // Make an input stream from the byte array and read
          // a copy of the object back in.
          ObjectInputStream in = new ObjectInputStream(
              new ByteArrayInputStream(bos.toByteArray()));
          obj = in.readObject();
      }
      catch(IOException e) {
          e.printStackTrace();
      }
      catch(ClassNotFoundException cnfe) {
          cnfe.printStackTrace();
      }
      return obj;
  }


}
