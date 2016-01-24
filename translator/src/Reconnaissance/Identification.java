package Reconnaissance;


import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.HashMap;
import java.util.Map;

public class Identification {

	public static String input = "le chat noir miaule";
	public static String exe = "/net/public/tal/moses/kenlm/query ";
	public static String ch = "/net/public/tal/identificationLangue/language-models/install.txt_utf8.";
	public static String finCh = ".3g.kn.lm 2";
	public static String cible = "/dev/null";
	public static Map <String,String> langues = new HashMap<String,String>();



	public static void main(String[] args) {
		//init map
		langues.put("fr", "francais");
		langues.put("en", "Anglais");
		langues.put("de", "Allemand");
		langues.put("ca", "Catalan");
		langues.put("cs", "Tchèque");
		langues.put("es", "Espagnol");
		langues.put("hu", "Hongrois");
		langues.put("pt", "Portugais");
		langues.put("ru", "Russe");
		langues.put("sv", "Suédois");

		float maximum = Float.MIN_VALUE;
		String l = "UNKNOWN";

		//parcous des langues
		for(Map.Entry<String, String> lang: langues.entrySet()) {
			//génération de la commande
			String cmd = exe + ch + lang.getKey() + finCh + " > " + cible;
			try {
				//création du processus et des buffers I/O
				Process process = Runtime.getRuntime().exec(cmd);
				OutputStreamWriter w = new OutputStreamWriter(process.getOutputStream());
				BufferedReader r = new BufferedReader(new InputStreamReader(process.getInputStream(), "UTF-8"));

				//envoie de la phrase dont on veut identifier la langue
				w.write("\""+input+ "\"\n");
				w.flush();

				//récupération du score
				String[] receivedSpl = r.readLine().trim().split("\\s");
				Float score = Float.valueOf(receivedSpl[receivedSpl.length-1]);

				if(score>maximum) {
					maximum = score;
					l = lang.getValue();
				}
				w.close();
				r.close();
				process.destroy();
			}
			catch (IOException e) {
				System.out.println("Impossible d'analyser la langue " + lang.getValue() + ".\n" + e.getMessage());
			}
		}//fin parcours
		//affichage du resultat

		System.out.println("La langue est le " + l + ".\n" +
				"La probabilité que ce soit juste est de : " + maximum + "%");
	}


}
