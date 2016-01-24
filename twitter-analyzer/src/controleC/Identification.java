package controleC;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import projet.Chaine;

public class Identification {

	
	 public static void main(String[] args) {
		     Map <String,String> langue = new HashMap<String,String>();
		     
		     langue.put("francais", "/net/public/tal/moses/kenlm/query " +
				"/net/public/tal/identificationLangue/language-models/install.txt_utf8.fr.3g.kn.lm 2> /dev/null");
		langue.put("Anglais", "/net/public/tal/moses/kenlm/query " +
		"/net/public/tal/identificationLangue/language-models/install.txt_utf8.en.3g.kn.lm 2> /dev/null");
		langue.put("de", "/net/public/tal/moses/kenlm/query " +
		"/net/public/tal/identificationLangue/language-models/install.txt_utf8.de.3g.kn.lm 2> /dev/null");
		langue.put("ca", "/net/public/tal/moses/kenlm/query " +
		"/net/public/tal/identificationLangue/language-models/install.txt_utf8.ca.3g.kn.lm 2> /dev/null");
		langue.put("cs", "/net/public/tal/moses/kenlm/query " +
		"/net/public/tal/identificationLangue/language-models/install.txt_utf8.cs.3g.kn.lm 2> /dev/null");
		langue.put("es", "/net/public/tal/moses/kenlm/query " +
		"/net/public/tal/identificationLangue/language-models/install.txt_utf8.es.3g.kn.lm 2> /dev/null");
		langue.put("hu", "/net/public/tal/moses/kenlm/query " +
		"/net/public/tal/identificationLangue/language-models/install.txt_utf8.hu.3g.kn.lm 2> /dev/null");
		langue.put("pt", "/net/public/tal/moses/kenlm/query " +
		"/net/public/tal/identificationLangue/language-models/install.txt_utf8.pt.3g.kn.lm 2> /dev/null");
		langue.put("ru", "/net/public/tal/moses/kenlm/query " +
		"/net/public/tal/identificationLangue/language-models/install.txt_utf8.ru.3g.kn.lm 2> /dev/null");
		langue.put("sv", "/net/public/tal/moses/kenlm/query " +
		"/net/public/tal/identificationLangue/language-models/install.txt_utf8.sv.3g.kn.lm 2> /dev/null");
		    float max = -1000000;
		    String lang = "";
		    float tmp = 0;
		    String inputLine = "aprés l'installation du systéme";
		    Iterator<String> it = langue.keySet().iterator();
		    while(it.hasNext()){
		    	String l = it.next();
		    	 
			        String cmd = langue.get(l);
			        
			        //System.out.println(cmd);
			        
			        try {
			        	
			            Process process = Runtime.getRuntime().exec(cmd);

			            OutputStreamWriter w = new OutputStreamWriter(
			                    process.getOutputStream());

			            BufferedReader r = new BufferedReader(
			                    new InputStreamReader(process.getInputStream(), "UTF-8"));

			            //BufferedReader in = new BufferedReader(
			            //        new InputStreamReader(System.in));
			           
			            //System.out.println("Veuillez entrer une phrase :");

			                        w.write("\""+inputLine+ "\"\n");
			                        w.flush();
			                        String received;
			                        received = r.readLine();
			                            received.trim();
			                            String[] line = received.split(" ");
			                            String score = line[line.length-1];
			                            tmp =  Float.parseFloat(score) ;
			                            if(tmp > max ) {
			                            	max = tmp;
			                            	lang = l;
			        			        }
			                        	System.out.println(l+": "+tmp);
			                           
			                        
			                
			               
			            w.close();
			            r.close();
			            //process.waitFor();
			            process.destroy();
			        } catch (IOException e) {
			            System.out.println(cmd + ": " + e.getMessage());
			            System.exit(1);
			        } catch (Exception e) {
			            System.out.println("Error: " + e.getMessage());
			            System.exit(1);
			        }
			        
	    		
	    		
	    	}
		    System.out.println("la langue est:"+lang+": "+max);
            
           
	    }
}
