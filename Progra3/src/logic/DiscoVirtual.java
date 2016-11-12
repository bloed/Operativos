package logic;

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
    root = new Nodo("root","root");
    sectores = new ArrayList<Sector>();
  }
  
  public String toString() {
    return "Cantidad Sectores: " + cantidadSectores + ". Tama�o de cada sector: " + tamanoSectores + 
        ". Tama�o total: " +  tamanoTotal + ".";
  }

}
