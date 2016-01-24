package projet;

import java.io.*;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import edu.stanford.nlp.trees.*;
import edu.stanford.nlp.trees.tregex.ParseException;
import edu.stanford.nlp.trees.tregex.TregexMatcher;
import edu.stanford.nlp.trees.tregex.TregexPattern;

public class Lexique {

    private static String filePos = "indexPos.txt";
    private static String fileNeg = "indexNeg.txt";

    public static ArrayList<String> initPositif () {
		ArrayList <String> positifs = new ArrayList <String> ();
		positifs.add("bon");
		positifs.add("bonne");
		positifs.add("agréable");
		positifs.add("beau");
		positifs.add("belle");
		positifs.add("charmant");
		positifs.add("charmante");
		positifs.add("super");
		positifs.add("génial");
		positifs.add("cool");
		positifs.add("fantastique");
		positifs.add("excellent");
		positifs.add("excellente");
		positifs.add("intéressant");
		positifs.add("intéressante");
		
		positifs.add("parfait");
		positifs.add("parfaite");
		positifs.add("félicitation");
		positifs.add("félicitations");
		positifs.add("rire");
		positifs.add("rires");
		positifs.add("jolie");
		positifs.add("jolies");
		positifs.add("plaisir");
		positifs.add("joyeux");
		positifs.add("joyeuse");
		positifs.add("barres");
		positifs.add("talent");
		positifs.add("sympa");
		positifs.add("meilleur");
		
		
		
		ArrayList<String> l = new ArrayList<String>(positifs.size());
		
		for(String s : positifs) {
			l.add(Chaine.removeAccent(s));
		}
		
		return l;
	}

    public static ArrayList<String> initNegatif () {
		ArrayList <String> negatifs = new ArrayList <String> ();
		negatifs.add("mauvais");
		negatifs.add("mauvaise");
		negatifs.add("desagreable");
		negatifs.add("mediocre");
		negatifs.add("pourri");
		negatifs.add("pourris");
		negatifs.add("nul");
		negatifs.add("nuls");
		negatifs.add("nulle");
		negatifs.add("nulles");
		negatifs.add("naze");
		negatifs.add("horrible");
		negatifs.add("horible");
		negatifs.add("affreuse");
		negatifs.add("affreu");
		negatifs.add("affreux");
		negatifs.add("affreuses");
		negatifs.add("ininterressante");
		negatifs.add("ininterressant");
		negatifs.add("pire");
		negatifs.add("mort");
		negatifs.add("morts");
		negatifs.add("mourir");
		
		negatifs.add("snif");
		negatifs.add("deçu");
		negatifs.add("decue");
		negatifs.add("degoute");
		negatifs.add("degoutee");
		negatifs.add("deprime");
		negatifs.add("craint");
		negatifs.add("deteste");
		negatifs.add("malheureusement");
		negatifs.add("ennui");
		negatifs.add("pleurer");
		negatifs.add("pleure");
		negatifs.add("pauvre");
		negatifs.add("manquer");
		negatifs.add("manque");
		negatifs.add("marre");
		negatifs.add("perdu");
		negatifs.add("désolé");
		negatifs.add("desolee");
		negatifs.add("desolant");
		negatifs.add("rate");
		negatifs.add("putain");
		negatifs.add("chiant");
		negatifs.add("merde");
		negatifs.add("ferme");
		negatifs.add("ouin");
		negatifs.add("chier");
		negatifs.add("taff");
		negatifs.add("loupé");
		negatifs.add("galere");
		negatifs.add("galeres");
		negatifs.add("mechant");
		negatifs.add("mechants");
		negatifs.add("bordel");
		negatifs.add("crise");
		negatifs.add("crises");
		negatifs.add("peur");
		negatifs.add("erreur");
		negatifs.add("erreures");
		negatifs.add("erreurs");
		negatifs.add("erreure");
		negatifs.add("molle");
		negatifs.add("mou");
		negatifs.add("timide");
		negatifs.add("molles");
		negatifs.add("mous");
		negatifs.add("timides");
		negatifs.add("paye");
		negatifs.add("payer");
		negatifs.add("boulo");
		negatifs.add("boulot");
		negatifs.add("honte");
		negatifs.add("tuer");
		negatifs.add("sauvagement");
		negatifs.add("rien");
		negatifs.add("meurtrier");
		negatifs.add("assassin");
		negatifs.add("atroce");
		negatifs.add("ruine");
		negatifs.add("trompe");
		negatifs.add("pourriture");
		negatifs.add("creve");
		
		
		
		ArrayList<String> l = new ArrayList<String>(negatifs.size());
		
		for(String s : negatifs) {
			l.add(Chaine.removeAccent(s));
		}
		return l;
	}
    /**
     *
     * @return Liste des patrons avec leur polarisation des lemmes vrai pour un "et", faux pour un "mais"
     */
    public static List<Doublet<String, Boolean>> initPatrons() {
        List<Doublet<String, Boolean>> patrons = new ArrayList<Doublet<String, Boolean>>();
        patrons.add(new Doublet<String, Boolean>("ADJ=adj1 . (COORD < (CC[<< et] . (AP < ADJ=adj2)))", true));
        patrons.add(new Doublet<String, Boolean>("ADJ=adj1 . (COORD < (CC[<< et] . (AP < (ADV . ADJ=adj2))))", true));
        patrons.add(new Doublet<String, Boolean>("AP < (ADV . ADJ=adj1) . (COORD < (CC[<< et] . (AP < ADJ=adj2)))", true));
        patrons.add(new Doublet<String, Boolean>("(AP < (ADV . ADJ=adj1)) . (COORD < (CC[<< et] . (VN . (NP < (DET . ADJ=adj2 . ADJ)))))", true));
        patrons.add(new Doublet<String, Boolean>("ADJ=adj1 . (COORD < (CC[<< et] . (VN . (NP < (DET . ADJ=adj2 . ADJ)))))", true));
        patrons.add(new Doublet<String, Boolean>("ADJ=adj1 . (COORD < (CC[<< et] . (VN . (AP < ADJ=adj2))) )", true));



        //négatifs
        patrons.add(new Doublet<String, Boolean>("ADJ=adj1 . (COORD < (CC[<< mais] . (AP < ADJ=adj2)))", false));
        patrons.add(new Doublet<String, Boolean>("ADJ=adj1 . (COORD < (CC[<< mais] . (AP < (ADV . ADJ=adj2))))", false));
        patrons.add(new Doublet<String, Boolean>("AP < (ADV . ADJ=adj1) . (COORD < (CC[<< mais] . (AP < ADJ=adj2)))", false));
        patrons.add(new Doublet<String, Boolean>("AP < (ADV . ADJ=adj1) . (COORD < (CC[<< mais] . (AP < ADJ=adj2)))", false));
        patrons.add(new Doublet<String, Boolean>("ADJ=adj1 . (COORD < (CC[<< mais] . (VN . (NP < (DET . ADJ=adj2 . ADJ)))))", false));
        patrons.add(new Doublet<String, Boolean>("ADJ=adj1 . (COORD < (CC[<< mais] . (VN . (AP < ADJ=adj2))) )", false));



        return patrons;
    }

    private static Doublet<List<String>, List<String>> extract(String inputFile, List<String> positifs, List<String> negatifs) {

        List<Doublet<String, Boolean>> patrons = initPatrons();


        TreeFactory treeFactory = new LabeledScoredTreeFactory();



        try {
            InputStream input = new FileInputStream(new File(inputFile));
            BufferedReader reader = new BufferedReader(new InputStreamReader(input, "UTF-8"));

            String line;

            while ((line = reader.readLine()) != null) {

                TreeReader treeReader = new PennTreeReader(new StringReader(line), treeFactory);
                Tree tree = treeReader.readTree();

                if (tree != null) {
                    for (Doublet<String, Boolean> d : patrons) {
                        TregexPattern tpattern = TregexPattern.compile(d.getFirst());
                        TregexMatcher tmatch = tpattern.matcher(tree);
                        if(d.getSecond()) { // ajout dans la meme polarite
							if(tmatch.find()) {
								String adj1 = tmatch.getNode("adj1").yield().get(0).value().toLowerCase();
								String adj2 = tmatch.getNode("adj2").yield().get(0).value().toLowerCase();
								
								System.out.println("adj ET= " + adj1 + " " + adj2);
								
								if(positifs.contains(adj1)) { if(!positifs.contains(adj2)) positifs.add(adj2); }
								if(positifs.contains(adj2)) { if(!positifs.contains(adj1)) positifs.add(adj1); }
								if(negatifs.contains(adj1)) { if(!negatifs.contains(adj2)) negatifs.add(adj2); }
								if(negatifs.contains(adj2)) { if(!negatifs.contains(adj1)) negatifs.add(adj1); }
								
							}
						}
						else {
							if(tmatch.find()) { // ajout dans la polarite inverse
								String adj1 = tmatch.getNode("adj1").yield().get(0).value().toLowerCase();
								String adj2 = tmatch.getNode("adj2").yield().get(0).value().toLowerCase();
								
								System.out.println("adj Mais= " + adj1 + " " + adj2);
								
								if(positifs.contains(adj1)) { if(!negatifs.contains(adj2)) negatifs.add(adj2); }
								if(positifs.contains(adj2)) { if(!negatifs.contains(adj1)) negatifs.add(adj1); }
								if(negatifs.contains(adj1)) { if(!positifs.contains(adj2)) positifs.add(adj2); }
								if(negatifs.contains(adj2)) { if(!positifs.contains(adj1)) positifs.add(adj1); }
							}
						}
                    }// fin du parcours des patrons
                }
            }//end while


            System.out.println("Ajectifs positifs : " + positifs.toString() + "\n\n\n");
            System.out.println("Ajectifs négatifs : " + negatifs.toString() + "\n\n\n");


        } catch (FileNotFoundException e) {
            System.out.println("Fichier non trouvé");
            e.printStackTrace();
        } catch (UnsupportedEncodingException e) {
            System.out.println("Fichier pas en UTF-8");
            e.printStackTrace();
        } catch (IOException e) {
            System.out.println("Erreur IO dans le readLine");
            e.printStackTrace();
        } catch (ParseException e) {
            System.out.println("Erreur de parsage a la compilation du pattern");
            e.printStackTrace();
        }

        return (new Doublet<List<String>, List<String>>(positifs, negatifs));

    }


	public static Map<String, Boolean> recursiveExtract (String inputFile) {
		Doublet<List <String>, List <String>> d = new Doublet<List <String>, List <String>>(initPositif (), initNegatif ());
		int maxPasses = 10;
		int currentPass = 0;
		int sizep = 0;
		int sizen = 0;
		Boolean b;
		do {
			sizep = d.getFirst().size();
			sizen = d.getSecond().size();
			d = extract(inputFile, d.getFirst(), d.getSecond());
			
		}while (sizep != d.getFirst().size() && sizen != d.getSecond().size() && currentPass++<maxPasses);
		
		
		
		Map<String, Boolean> hash = new Hashtable<String, Boolean>(d.getFirst().size() + d.getSecond().size());
		for(String s : d.getFirst()) {
			hash.put(s, true);
		}
		for(String s : d.getSecond()) {
			b = hash.put(s, false);
			if(b!=null) {
				System.out.println("Clef aussi positive : " + s);
				hash.remove(s);
			}
		}
		
		
		
		return hash;
	}

	public static Map<String, Boolean> loadLexique () {
		//Doublet<List <String>, List <String>> d = new Doublet<List <String>, List <String>>(new ArrayList<String>(), new ArrayList<String>());
		Map <String, Boolean> hash = new Hashtable<String, Boolean>();
		try {
			FileReader fr = new FileReader(filePos);
			BufferedReader input = new BufferedReader(fr);
			String s;
			while((s = input.readLine()) != null) {
				hash.put(s, true);
			}
			input.close();
			
			fr = new FileReader(fileNeg);
			input = new BufferedReader(fr);
			while((s = input.readLine()) != null) {
				hash.put(s, false);
			}
			input.close();
			
			return hash;
			
		} catch (FileNotFoundException e) {
			System.out.println("Impossible de charger les fichiers de lexique");
			e.printStackTrace();
			return hash;
		} catch (IOException e) {
			System.out.println("Erreur de lecture");
			e.printStackTrace();
			return hash;
		}
	}
	
	public static Map<String, Boolean> makeAndWriteLexique (String inputFile) {
		Map<String, Boolean> map = recursiveExtract(inputFile);
		Set<Map.Entry<String, Boolean>> set = map.entrySet();
		Iterator<Entry<String, Boolean>> it = set.iterator();
		List<String> lPos = new ArrayList<String>();
		List<String> lNeg = new ArrayList<String>();
		while(it.hasNext()) {
			Entry<String, Boolean> e = it.next();
			if(e.getValue()) {
				lPos.add(e.getKey());
			}
			else {
				lNeg.add(e.getKey());
			}
		}
		
		System.out.println(lNeg.size());
		try {
			FileWriter fw = new FileWriter(filePos, false);
			BufferedWriter output = new BufferedWriter(fw);
			for(String s : lPos) {
				output.write(s);
				output.newLine();
			}
			output.flush();
			output.close();
			
			fw = new FileWriter(fileNeg, false);
			output = new BufferedWriter(fw);
			for(String s : lNeg) {
				output.write(s);
				output.newLine();
			}
			output.flush();
			output.close();
			
		} catch (IOException e) {
			System.out.println("Impossible d'ecrire le lexique");
			e.printStackTrace();
		}
		return map;
	}

    public static void main(String arg[]) {
        makeAndWriteLexique("arbres.txt");
    }
}
