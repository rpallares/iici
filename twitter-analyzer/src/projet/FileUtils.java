/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package projet;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

/**
 *
 * @author odonroch
 */
public class FileUtils {

    public static void copy(String fileIn, String fileOut) throws FileNotFoundException, IOException {
     
            File inputFile = new File(fileIn);
            File outputFile = new File(fileOut);

            FileReader in = new FileReader(inputFile);
            FileWriter out = new FileWriter(outputFile);
            int c;

            while ((c = in.read()) != -1) {
                out.write(c);
            }

            in.close();
            out.flush();
            out.close();


            System.out.println("Copie réussie");
    }

    public static void delete(String fileName) throws IllegalArgumentException {
        // A File object to represent the filename
        File f = new File(fileName);

        // Make sure the file or directory exists and isn't write protected
        if (!f.exists()) {
            throw new IllegalArgumentException(
                    "Aucun fichier ou dossier: " + fileName);
        }

        if (!f.canWrite()) {
            throw new IllegalArgumentException("Suppression impossible car le fichier protégé en écriture: "
                    + fileName);
        }

        // If it is a directory, make sure it is empty
        if (f.isDirectory()) {
            String[] files = f.list();
            if (files.length > 0) {
                throw new IllegalArgumentException(
                        "Impossible de supprimer le dossier car il n'est pas vide: " + fileName);
            }
        }

        // Attempt to delete it
        boolean success = f.delete();

        if (!success) {
            throw new IllegalArgumentException("Echec de la suppression du fichier");
        }
    }
}
