package Reconnaissance;


import java.util.LinkedList;

/**
 * Cette classe permet de décrire un terme avec le terme
 * son genre (N, V, ...)
 * et ses traductions possibles (trouvées dans un dictionnaire)
 * @author rafael pallares
 */
public class Entree {

	private String mot;
	private String genre;
	private LinkedList<String> traductions;
	
	public Entree(String m,String g,LinkedList<String> t){
		
		mot = m;
		genre = g;
		traductions = t;

	}
	public String getMot(){
		return mot;
	}
	public String getGenre(){
		return genre;
	}
	public LinkedList<String> getTraduction(){
		return traductions;
	}
}
