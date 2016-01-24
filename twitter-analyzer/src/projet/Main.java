/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package projet;

import java.io.IOException;

/**
 *
 * @author odonroch
 */
public class Main {

    public static void main(String arg[]) throws IOException {
        MakeCorpus.main(arg);
        
        System.out.print("Correcting...");
        TwitterSpellChecker.main(arg);
        System.out.println("OK !");
        
        Format_corpus.main(arg);
        
        //System.out.print("Running BONSAI...");
        //Runtime.getRuntime().exec("sh /net/public/tal/bonsai_v3.2/bin/bonsai_bky_parse_via_clust.sh corpus_correct.txt > arbres.txt");
        //System.out.println("OK !");
        
        
        //Lexique.main(arg);
        //Opinion.main(arg);
    }

}
