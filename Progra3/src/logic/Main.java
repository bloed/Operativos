package logic;

import java.util.Scanner;

public class Main {
  

  public static Scanner in = new Scanner(System.in);
  public static DiscoVirtual discoVirtual;


  public Main() {
    
  }

  public static void main(String[] args) {
    System.out.println("Bienvenidos");
    crearDiscoVirtual();
    String s;
    String result;
    while(true){
      imprimirMenu();
      s = getInputString();
      if (s.equals("0")){
        break;
      }
      result = FileSystem.action(s, null, null);
      System.out.println(result);
    }
    System.out.println("Revisar archivo en disco!");
  }
  
  public static void crearDiscoVirtual(){
    System.out.println("Primero se debe crear un disco virtual con la funcion CREATE");
    System.out.println("Ingresa la cantidad de sectores:");
    Integer cantidad = getInputInt();
    System.out.println("Ingresa el tamaño de cada sector (bytes)");
    Integer tamano = getInputInt();
    discoVirtual = new DiscoVirtual(cantidad, tamano);
    System.out.println(discoVirtual.toString());
    getInputString();
  }
  
  public static void imprimirMenu(){
    System.out.println("Para salir : 0.");
    System.out.println("Ingresa la acción que se desea realizar (mismo nombre que en la especificación):");
  }
  
  public static String getInputString(){
    String s = in.nextLine();
    return s;
  }
  public static Integer getInputInt(){
    Integer i = in.nextInt();
    return i;
  }

}
