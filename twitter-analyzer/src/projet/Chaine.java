package projet;

import java.nio.charset.Charset;
import java.text.Normalizer;

public class Chaine {

    public static String removeAccent(String source) {
        return Normalizer.normalize(source, Normalizer.Form.NFD).replaceAll("[\u0300-\u036F]", "");
    }

    public static String toUTF8(String s) {
        return new String(s.getBytes(), Charset.forName("UTF8"));
    }
}