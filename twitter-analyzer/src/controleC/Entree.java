package controleC;

import java.util.List;

public class Entree {

	private String mot;
	private String genre;
	private List<String> traductions;
	
	public Entree(String m,String g,List<String> t){
		
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
	public List<String> getTraduction(){
		return traductions;
	}
	//{}
}
