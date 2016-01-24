package controleC;
import java.io.*;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import java.util.*;


public class Traduction {
	
	private Map<String, Entree> dico = new HashMap<String, Entree>();
	
	
	public Map<String,Entree> getDico() throws IOException{
		//Traduction t = new Traduction();
		BufferedReader in = new BufferedReader(new FileReader("dico_fr-en_NAPD.txt"));
	    String inputLine;
	    
	    
	    while ( (inputLine = in.readLine()) != null ) {
	    	inputLine.trim();
	    	String[] motg = inputLine.split(" ");
	    	
	    	String m = motg[0];
	    	String g = motg[1];
	    	g = normaliseC(g);
	    	
	    	inputLine = normalise(inputLine);
	    	//prendre la chaine apr�s la premiere parenth�se
	    	inputLine = inputLine.substring(inputLine.indexOf(')')+1);
	    	String[] line = inputLine.split("@");
	    	List<String> tr = new ArrayList<String>();
	    	
	    	for(int i=0; i< line.length; i++){
	    		
	    		
	    		if(line[i].contains("(") ){
	    			String str = line[i].substring(0,line[i].indexOf('(')-1 )+
	    			line[i].substring(line[i].indexOf(')')+1 );
	    			tr.add(str);
	    		   
				}
	    		else {tr.add(line[i]);}
	    		
	    		  
	    		
	    	}
	    	dico.put(m, new Entree(m,g,tr));
	    	
	    	
		}
	    return dico;
	}
	
	
	
	public String normalise(String g){
		String res = "";
		for(int i=0;i< g.length();i++){
			
			if(g.charAt(i)== '|'){
				res+='@';
				
			}
			else res+=g.charAt(i);
		}
		
		return res;
	}
	
	
	public String normaliseC(String g){
		String res = "";
		for(int i=0;i< g.length();i++){
			
			if((g.charAt(i) != '(')&&(g.charAt(i) != ')')){
				res+=g.charAt(i);
				
			}
			
		}
		
		return res;
	}

	
	
	
	public void traduire(String s,String[] l, int indice, Map<String,Entree> d) throws IOException{
		
		  Traduction t = new Traduction();
	      String mot = l[indice];
	      
	       Entree e = d.get(mot);
	       String tmp = s;
			 for(int j=0;j< e.getTraduction().size();j++){
				 s += " "+e.getTraduction().get(j);
				 if(indice < l.length-1) { 
					 t.traduire(s,l,indice+1,d);
				     s = tmp;
				     }
				 else{ 
					 System.out.println(s);
				     s =tmp;
				     }
				 
			 }
			 
	}
	
	
public static void main(String[] argv)throws IOException
{
		Traduction t = new Traduction();
		//Configuration du son
		String terme = "abandon abats son";
		
	    Map<String, Entree> d = new HashMap<String, Entree>();
	    d = t.getDico();
       String[] tab_terme = terme.split(" ");
       int debut = 0;
       String mot = tab_terme[debut];
       Entree e = d.get(mot);
       t.traduire("",tab_terme,0,d);
	
		
	}
	
}
