package entrepot;

import entrepot.OlapRequete.Operator;

public class Contrainte {

    public class MalformedRequestException extends Exception {

        private static final long serialVersionUID = 1L;

        public MalformedRequestException() {
            super();
        }
    }
    private int operande;
    private String operator;
    private String valeur;

    public Contrainte(int opd, String o, String val) {
        operande = opd;
        operator = o;
        valeur = val;

    }

    public int getOperande() {
        return operande;
    }

    public String getOperator() {
        return operator;
    }

    public String getValeur() {
        return valeur;
    }
}
