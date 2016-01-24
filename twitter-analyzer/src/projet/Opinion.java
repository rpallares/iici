package projet;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import edu.stanford.nlp.trees.*;
import edu.stanford.nlp.trees.tregex.ParseException;
import edu.stanford.nlp.trees.tregex.TregexMatcher;
import edu.stanford.nlp.trees.tregex.TregexPattern;

public class Opinion {
	
	static private int seuil = 1;
	
	public static List<String> loadVerbs () {
		List<String> list = new ArrayList<String>();
		list.add("abhorrer");
		list.add("agacer");
		list.add("aimer");
		list.add("kiffer");
		list.add("kiffer");
		list.add("adorer");
		list.add("adorer");
		list.add("apprécier");
		list.add("décevoir");
		list.add("déplorer");
		list.add("détester");
		list.add("déranger");
		list.add("désappointer");
		list.add("exaspérer");
		list.add("excéder");
		list.add("exécrer");
		list.add("énerver");
		list.add("gêner");
		list.add("haïr");
		list.add("horripiler");
		list.add("idolâtrer");
		list.add("mépriser");
		list.add("regretter");
		list.add("vénérer");
		
		List<String> l2 = new ArrayList<String>(2*list.size());
		for(String s : list) {
			String s2 = Chaine.removeAccent(s);
			l2.add(s2);
			l2.add(s2.substring(0, s2.length()-1));
		}
		
		return l2;
	}
	
	/**
	 * retourne la liste des doublets des patrons d'opinions
	 * Si n>0 <==> nb adjectifs a analyser
	 * si n==0 patron positif
	 * si n==-1 patron negatif
	 * si n==-2 mixte
	 * @return
	 */
	public static List<Doublet<String, Integer>> loadSubjPatron () {
		List<Doublet<String, Integer>> l = new ArrayList<Doublet<String, Integer>>();
		l.add(new Doublet<String,Integer>("/c'|C'|ce|Ce/ . (/est|etait|sera|fut/ . (ADV . (ADV . ADJ=adj1)))",1));
		l.add(new Doublet<String,Integer>("/c'|C'|ce|Ce/ . (/est|etait|sera|fut/ . (ADV . ADJ=adj1))",1));
		l.add(new Doublet<String,Integer>("/c'|C'|ce|Ce/ . (/est|etait|sera|fut/ . ADJ=adj1)",1));
		l.add(new Doublet<String,Integer>("DET . (ADJ=adj1 . NC)",1));
		l.add(new Doublet<String,Integer>("GN . (V[<<etre |<<est |<<etait |<<fut |<<sera] . ADJ=adj1)",1));
		l.add(new Doublet<String,Integer>("V < /^ador.*/ ",0));
		l.add(new Doublet<String,Integer>("(V < /^aim.*/) . !/pas|plus/",0));
		l.add(new Doublet<String,Integer>("(V < /^aim.*/) . /pas|plus/",-1));
		l.add(new Doublet<String,Integer>("/^kiff*/ . !/pas|plus/",0));
		l.add(new Doublet<String,Integer>("/^kiff*/ . /pas|plus/",-1));
		l.add(new Doublet<String,Integer>("/^(O|o)(u|U)(i|I)(i|I)+/",0));
		l.add(new Doublet<String,Integer>("/^(N|n)(a|o|A|O)(a|o|A|O)+(n|N)(n|N)+/",-1));
		l.add(new Doublet<String,Integer>("/^(ha|Ha|HA){2,}/",0));
		l.add(new Doublet<String,Integer>("/N'|n'/ . (importe . quoi)",-1));
		l.add(new Doublet<String,Integer>("/N'|n'/ . importequoi",-1));
		l.add(new Doublet<String,Integer>("/trop|tres/ . (ADJ=adj1 . NP)",1));
		l.add(new Doublet<String,Integer>("moi . (/j'|je/ . V)",-2));
		l.add(new Doublet<String,Integer>("est . ADJ=adj1",1));
		l.add(new Doublet<String,Integer>("/</ . /3/",0));
		l.add(new Doublet<String,Integer>("CLS . (CLR . (/en/ . /fout/))",-2));
		l.add(new Doublet<String,Integer>("/putainn+|PUTAINN+/",-1));
		l.add(new Doublet<String,Integer>("/non/ . (/mais/ . /serieu.*/)",-1));
		l.add(new Doublet<String,Integer>("/fera|faire|feront/ . /payer/",-1));
		l.add(new Doublet<String,Integer>("DET . (NC . ADJ=adj1)",1));
		
		return l;
	}
	
	public static Doublet<Boolean, Orientation> classerTweet (String tweet, Map<String, Boolean> map, TreeFactory treeFactory) {
		Triplet<Boolean, Integer, Integer> t = isSubjectif(tweet, map, treeFactory);
		
		Boolean isSubj = t.getFirst();
		int nbPos = t.getSecond();
		int nbNeg = t.getThird();
		
		Boolean b;
		
		tweet = tweet.substring(2);
		tweet = tweet.replaceAll("\\((SENT|NP|ADJ|NC|PONCT|AP|ADV|VN|V|PP|P|CLS|Ssub|CS|Sint|CLR|DET|COORD|CC|PRO|Srel|PROPEL|CLO|VPP|P\\+D|VPR|VPinf|VINF|ET|NPP|PROWH|I|AdP|VPpart|VIMP|PROREL|VS|ADVWH) ", "");
		tweet = tweet.replaceAll("\\)", "");
		
		for(String mot : tweet.split(" +")) {
			if((b = map.get(mot)) != null) {
				if(b) { nbPos++; }
				else { nbNeg++; }
			}
		}
		//System.out.println("Tweet : " + tweet + "\n\tPos : " + nbPos + " Neg : " + nbNeg);
		
		if (isSubj) {
            System.out.println("tweet : " + tweet + "\n\tPos : " + nbPos + " Neg : " + nbNeg);
			if(nbPos > nbNeg) { return new Doublet<Boolean, Orientation>(true,Orientation.POSITIF); }
			if(nbPos < nbNeg) { return new Doublet<Boolean, Orientation>(true,Orientation.NEGATIF); }
			return new Doublet<Boolean, Orientation>(true,Orientation.MIXTE);
		}
		else {
			return new Doublet<Boolean, Orientation>(false,Orientation.INDEFINI);
		}
	}
	
	private static Triplet<Boolean, Integer, Integer> isSubjectif (String tweet, Map<String, Boolean> map, TreeFactory treeFactory) {
		int nbSubj = 0, nbPos = 0, nbNeg = 0;
		for(String s : loadVerbs()) {
			if(tweet.contains(s))
				nbSubj++;
		}
		
		//analyse des patrons
		TreeReader treeReader = new PennTreeReader(new StringReader(tweet), treeFactory);
		try {
			Tree tree = treeReader.readTree();
			for(Doublet<String, Integer> d : loadSubjPatron()) {
				if(tree != null) {
					String p = d.getFirst();
					//System.out.println(p);
					TregexPattern tpattern = TregexPattern.compile(p);
					TregexMatcher tmatch = tpattern.matcher(tree);
					
					while(tmatch.findNextMatchingNode()) {
						switch (d.getSecond()) {
						case -2 : // mixte, rien a faire
							break;
						case -1 : nbNeg++;
							break;
						case 0 : nbPos++;
							break;
						default :// > 0 adj a analyser
							int nbAdj = d.getSecond();
							String adj = tmatch.getNode("adj1").yield().get(0).value().toLowerCase();;
							
							Boolean b = map.get(adj);
							System.out.println("MATCH " + adj + "   " + b);
							if(b != null) {
								if(b) { nbPos+=2; }
								else { nbNeg+=2; }
							}
							
							for(int cAdj = 2; cAdj<nbAdj+1; cAdj++) {
								adj = tmatch.getNode("adj"+cAdj).yield().get(0).value().toLowerCase();;
								
								b = map.get(adj);
								if(b != null) {
									if(b) { nbPos++; }
									else { nbNeg++; }
								}
							}
							break;
						}
						nbSubj++;
					}
				}
			}
			
			if(nbSubj>=seuil) return new Triplet<Boolean, Integer, Integer>(true, nbPos, nbNeg);
			return new Triplet<Boolean, Integer, Integer>(false, 0, 0);
			
		} 
		catch (IOException e) {return new Triplet<Boolean, Integer, Integer>(false, 0, 0);} 
		catch (ParseException e) {
			e.printStackTrace();
			return new Triplet<Boolean, Integer, Integer>(false, 0, 0);
		}
	}
	
	
	public static void main (String [] args) {
		int nbSubj = 0, total = 0, nbPos = 0, nbNeg = 0, nbMixte = 0;
		Map <String, Boolean> map = Lexique.loadLexique();
		//System.out.println("taille map = " + map.size());
		TreeFactory treeFactory = new LabeledScoredTreeFactory();
		
		
		try {
			BufferedReader input = new BufferedReader(new FileReader("arbres.txt"));
			String tweet;
			
			
			while((tweet = input.readLine()) != null) {
				Doublet<Boolean, Orientation> d = classerTweet(tweet, map, treeFactory);
				if(d.getFirst()) {
					switch (d.getSecond()) {
					case POSITIF : nbPos++;
						break;
					case NEGATIF : nbNeg++;
						break;
					case MIXTE : //System.out.println("MIXTE : " + tweet);
						nbMixte++;
						break;
					case INDEFINI : System.out.println("Erreur, tweet subjectif non défini");
						break;
					default : break;
					}
					nbSubj++;
				}
				total++;
			}
			
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		System.out.println("Total tweets : " + total + ", dont " + nbSubj + " sont subjectifs.");
		System.out.println("     Tweets positifs : " + nbPos);
		System.out.println("     Tweets négatifs : " + nbNeg);
		System.out.println("     Tweets mixtes : " + nbMixte);
	}
}
