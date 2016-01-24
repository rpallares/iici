package Reconnaissance;


public class Machaine {
	String chain;
	String[] mot_u;
	int nb_mots;
	public Machaine(String c){
		this.chain =c ;
		 this.mot_u = chain.split(" ");
		 this.nb_mots=this.mot_u.length;
	}

}
