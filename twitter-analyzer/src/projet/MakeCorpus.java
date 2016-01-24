package projet;

import java.io.*;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import twitter4j.*;

public class MakeCorpus {
		private static String makeRequete () {
			List<String> verbs = Opinion.loadVerbs();
			verbs.addAll(Lexique.initNegatif());
			verbs.addAll(Lexique.initPositif());
			Collections.shuffle(verbs);
			int max = 30;
			String req = "";
			for(int i = 0; i<max && i<verbs.size();i++) {
				String s = verbs.get(i);
				req += s + " OR ";
			}
			
			req = req.substring(0, req.length()-3);
			req += "et OR mais ";
                        //req += "AND ";
			return req;
		}
		
		public static void main(String arg[]) throws IOException{
			
			FileWriter fw = new FileWriter("corpus.txt",false);
			BufferedWriter output = new BufferedWriter(fw);
			
			//envoyer vers le fichier
			Twitter twitter = new TwitterFactory().getInstance();
			String req = "revolution";
			String qString = makeRequete() + req;
                        //System.out.println(qString);
			Query query = new Query(qString);
			//Nombre de r�sultats � retourner = 100 (maximum)
                        query.setRpp(100);
			//Langue des tweets = fran�ais
			query.setLang("fr");
			QueryResult result;
			try {
				result = twitter.search(query);
				for (Tweet tweet : (ArrayList<Tweet>)result.getTweets()) {
				//	//System.out.println(tweet.getText());
					output.write(tweet.getText()+"\n");
				}
				query = new Query(req);
				query.setRpp(100);
				query.setLang("fr");
				result = twitter.search(query);
				for (Tweet tweet : (ArrayList<Tweet>)result.getTweets()) {
					//System.out.println(tweet.getText());
					output.write(tweet.getText()+"\n");
				}
			} catch (TwitterException e) {
			e.printStackTrace();
			}
			output.flush();
			output.close();
			System.out.println("Fichier corpus.txt créé.... Done");
			
		}

}
