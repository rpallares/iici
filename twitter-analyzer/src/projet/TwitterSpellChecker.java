package projet;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.text.CharacterIterator;
import java.text.StringCharacterIterator;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class TwitterSpellChecker {

    public static void main(String[] args) {

        String cmd = "hunspell -d fr_FR -a";
        String newWord = "";
        ArrayList <String> aExclure = new ArrayList<String>();

        try {

            Process process = Runtime.getRuntime().exec(cmd);

            OutputStreamWriter w = new OutputStreamWriter(
                    process.getOutputStream());

            BufferedReader r = new BufferedReader(
                    new InputStreamReader(process.getInputStream(), "UTF-8"));

            //BufferedReader in = new BufferedReader(
            //        new InputStreamReader(System.in));

            aExclure.add("lol");
            aExclure.add("mdr");
            aExclure.add("ptdr");
            aExclure.add("wahou");
            aExclure.add("email");
            aExclure.add("ftw");
            aExclure.add("wow");
            aExclure.add("savapa");
            aExclure.add("http");
            aExclure.add("com");

            FileWriter fw = new FileWriter("corpus_correct.txt", false);
            BufferedWriter output = new BufferedWriter(fw);
            BufferedReader in = new BufferedReader(new FileReader("corpus.txt"));

            String inputLine;
            //System.out.println("Veuillez entrer une phrase :");

            int i=0;

            while ((inputLine = in.readLine()) != null) {
                inputLine = Chaine.removeAccent(inputLine);
                inputLine.trim();
                String[] inputLineSpl = inputLine.split("[ \\.'/:!;,]");
                for (String word : inputLineSpl) {
                    Pattern p = Pattern.compile("[@#:].*");
                    Matcher m = p.matcher(word);

                    if (!(m.matches() || aExclure.contains(word))) {
                        w.write("\"" + word + "\"\n");
                        w.flush();
                        String received;
                        while ((received = r.readLine()) != null) {
                            received = received.trim();
                            if (received.length() == 0) {
                                break;
                            }
                            else if(received.compareTo("@(#) International Ispell Version 3.2.06 (but really Hunspell 1.2.11)") != 0) {
                                if (received.startsWith("&")) {
                                    String[] receivedSpl = received.split("(, |: )");
                                    if (receivedSpl.length > 1) {
                                        newWord = receivedSpl[1];
                                        output.write(newWord + " ");
                                    //System.out.println("newWord = " + newWord);
                                    }
                                    else {
                                        output.write(word + " ");
                                        //System.out.println("word1 = " + word);
                                    }
                                } else {
                                    output.write(word + " ");
                                    //System.out.println("word2 = " + word);
                                }
                            
                            }
                        }
                    } else {
                        output.write(word + " ");
                    }
                    //output.write(newWord + " ");
                    //System.out.print(newWord + " ");
                }
                output.write("\n");
            }
            output.flush();
            w.close();
            r.close();
            process.destroy();
        } catch (IOException e) {
            System.out.println(cmd + ": " + e.getMessage());
            System.exit(1);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
            System.exit(1);
        }
    }

    public static String forRegex(String aRegexFragment) {
        final StringBuilder result = new StringBuilder();

        final StringCharacterIterator iterator =
                new StringCharacterIterator(aRegexFragment);
        char character = iterator.current();
        while (character != CharacterIterator.DONE) {
            /*
            All literals need to have backslashes doubled.
             */
            if (character == '.' || character == '\\' || character == '?' || character == '*' ||
                    character == '+' || character == '&' || character == ':' ||
                    character == '{' || character == '}' || character == '[' || character == ']' ||
                    character == '(' || character == ')' || character == '^' || character == '$') {
            } else {
                //the char is not a special one
                //add it to the result as is
                result.append(character);
            }
            character = iterator.next();
        }
        return result.toString();
    }
}
