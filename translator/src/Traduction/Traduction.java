package Traduction;


import java.io.*;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import Reconnaissance.Entree;


public class Traduction {

	public static String exe = "/net/public/tal/moses/kenlm/query /net/public/tal/identificationLangue/language-models/install.txt_utf8.en.3g.kn.lm 2 > /dev/null";
	

	public Map<String,Entree> getDico() throws IOException{
		Map<String, Entree> dico = new HashMap<String, Entree>();
		BufferedReader in = new BufferedReader(new FileReader("dico_fr-en_NAPD.txt"));
		String inputLine;

		//Pattern pattern = Pattern.compile("^([\\w|-]+) (\\[.*\\] )?(\\([A-Z]+\\))(([ .*(\\(.*\\))? | .*(\\(.*\\))? \\|])+)$");
		//Pattern pattern = Pattern.compile("^(.*) (\\([A-Z]+\\))([ [\\p{Graph}| ]+( \\([^\\)]*\\))? | [\\p{Graph}| ]+( \\([^\\)]+\\))? \\|])+$");
		Pattern pattern = Pattern.compile("^(.*) (\\([A-Z]+\\))(.*)$");
		while ( (inputLine = in.readLine()) != null ) {
			Matcher m = pattern.matcher(inputLine);
			if(m.matches()) {
				String tmp = m.group(3).replaceAll("\\([^\\)]*\\)", "");
				tmp = tmp.replaceAll("\\[[^\\]]*\\]", "");
				String [] spl = tmp.split("\\|");

				LinkedList<String> l = new LinkedList<String>();
				for(String s : spl) {
					l.add(s.trim());
				}

				Entree e = new Entree(m.group(1), m.group(2).substring(1, m.group(2).length()-1), l);
				dico.put(m.group(1), e);
			}
		}
		return dico;
	}



	public static LinkedList<LinkedList<String>> generateAllComb(LinkedList<LinkedList<String>> l) {
		LinkedList<LinkedList<String>> lres= new LinkedList<LinkedList<String>>();
		//System.out.println(l);
		if(l.isEmpty()) return lres; //liste vide
		if(l.size() == 1) { // on doit créer n listes
			for(String s : l.pop()) {
				LinkedList<String> ltmp = new LinkedList<String>();
				ltmp.add(s);
				lres.add(ltmp);
			}
			return lres;
		}
		//appel reccurssif
		LinkedList<String> firsts = l.pop();
		LinkedList<LinkedList<String>> k = generateAllComb(l);
		
		for(String x : firsts) {//produit cartésien
			for(LinkedList<String> y : k) {
				LinkedList<String> ltmp = new LinkedList<String> ();
				ltmp.add(x);
				ltmp.addAll(y);
				lres.add(ltmp);
			}
		}
		return lres;

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


	public static void main(String[] argv)throws IOException {
		Traduction t = new Traduction();
		//Configuration du son
		String terme = "manger des pomme mur";
		
		Map<String, Entree> dico = new HashMap<String, Entree>();
		dico = t.getDico();
		LinkedList<LinkedList<String>> l = new LinkedList<LinkedList<String>>();
		Entree e;
		for(String m : terme.split(" ")) {
			if((e = dico.get(m)) != null) {
				l.add(e.getTraduction());
			}
		}
		
		l = Traduction.generateAllComb(l);
		
		Process process = Runtime.getRuntime().exec(exe);
		OutputStreamWriter w = new OutputStreamWriter(process.getOutputStream());
		BufferedReader r = new BufferedReader(new InputStreamReader(process.getInputStream(), "UTF-8"));
		
		Map<Float, String> map = new LinkedHashMap<Float,String>(l.size());
		float max = Float.MIN_VALUE;
		
		for(List<String> li : l) {
			String chaine = "";
			for (String s : li) {
				chaine += s + " ";
			}
			
			w.write("\""+chaine+ "\"\n");
			w.flush();
			String[] receivedSpl = r.readLine().trim().split("\\s");
			float score = Float.valueOf(receivedSpl[receivedSpl.length-1]);
			if(score>max) max=score;
			map.put(score, chaine);
		}
		
		System.out.println("La traduction la plus vraissemblable pour : " + terme);
		System.out.println("Score : " + max);
		System.out.println(map.get(max));


	}

}
