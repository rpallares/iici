package projet;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

public class Format_corpus {

    public static void main(String[] args) {
        try {
            boolean b=false;
            BufferedReader input ;
            BufferedWriter output ;
        	if(b) { //pas de spellchecker
        		input = new BufferedReader(new FileReader("corpus.txt"));
                output = new BufferedWriter(new FileWriter("corpus_correct.txt"));
        	}
        	else {
        		input = new BufferedReader(new FileReader("corpus_correct.txt"));
                output = new BufferedWriter(new FileWriter("corpus_formated.txt"));
        	}


            String tweet;
            while ((tweet = input.readLine()) != null) {
                tweet = Chaine.removeAccent(tweet);
                tweet = tweet.toLowerCase();
                tweet = tweet.replaceAll("(\\p{Punct}){2,}", ".");
                output.write(tweet + "\n");
            }
            input.close();
            output.flush();
            output.close();

            //Runtime.getRuntime().exec("");

            System.out.println("Done");


            if(!b) {
	            //FileUtils.delete("corpus.txt");
	            FileUtils.copy("corpus_formated.txt", "corpus_correct.txt");
	            FileUtils.delete("corpus_formated.txt");
            }


        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }



    }
}
