package entrepot;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.*;

import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;

public class OlapArbre implements TreeModel, Serializable {

    private static final long serialVersionUID = 4525312318363869647L;

    public class NomExistantException extends Exception implements Serializable {

        private static final long serialVersionUID = 4525312318363869647L;

        public NomExistantException() {
            super();
        }
    }
    private List<OlapArbre> fils; // permet d'avoir un indice des fils pr reconstituer le chemin
    private OlapArbre pere;
    private String nom;
    private int id;
    private SQLType monType;
    static private List<TreeModelListener> treeModListList;
    private static int cptInst = 0;
    public static Map<Integer, OlapArbre> hashTable = new HashMap<Integer, OlapArbre>();

    public OlapArbre(String _nom, SQLType t, List<OlapArbre> _fils) throws NomExistantException {
        nom = _nom;
        pere = null;
        fils = new ArrayList<OlapArbre>(_fils);
        id = cptInst++;
        monType = t;
        treeModListList = new ArrayList<TreeModelListener>();
        if (hashTable.containsValue(nom)) {
            throw new NomExistantException();
        }
        hashTable.put(id, this);
        for (OlapArbre f : fils) {
            f.pere = this;
        }
    }

    public OlapArbre(String _nom, SQLType t) throws NomExistantException {
        this(_nom, t, new ArrayList<OlapArbre>());
    }

    public OlapArbre(String _nom, SQLType t, OlapArbre a) throws NomExistantException {
        this(_nom, t);
        this.ajouterFils(a);
    }

    public void ajouterFils(OlapArbre f) {
        fils.add(f);
        f.pere = this;
    }

    /**
     * A TESTER
     * parcours largeur de l'arbre et supprime la branche dont le noeud a pour id _id
     * @param _id l'id du noeud a supprimer
     * @return true si la branche a �t� supprim�, false si non trouv�
     */
    public boolean supprimerFils(int _id) {
        if (fils.size() == 0) {
            return false; //feuille
        }
        for (int i = 0; i < fils.size(); i++) {
            if (fils.get(i).id == _id) {
                fils.remove(i);
                hashTable.remove(_id);
                return true;
            }
        }
        for (int i = 0; i < fils.size(); i++) {
            if (fils.get(i).supprimerFils(_id)) {
                return true;
            }
        }
        return false;
    }

    /**
     * parcours profondeur et retourne le vecteur des id des noeuds, dernier element=_id
     * @param _id l'id du noeud vers lequel on doit trouver le chemin
     * @return
     * @return retourne le chemin jusqu'a _id, ou un vecteur vide
     */
    public List<Integer> getChemin(int _id) {
        for (int i = 0; i < fils.size(); i++) {
            if (fils.get(i).id == _id) {
                List<Integer> lres = new LinkedList<Integer>();
                lres.add(_id);
                lres.add(id);
                return lres;
            }
            List<Integer> lres = getChemin(_id);
            if (!lres.isEmpty()) {
                lres.add(id);
                return lres;
            }
        }
        return new LinkedList<Integer>();
    }

    /**
     * parcours profondeur et forme la map des noms et SQLType de chaque noeud fils
     * s'utilise dans toSQLStarSchema
     * @param a arbre premier fils en parametre
     * @return la map des noms et SQLType de chaque noeud fils
     */
    static private List<Doublet<String, SQLType>> getMapNameSQLType(OlapArbre a) {
        List<Doublet<String, SQLType>> l = new ArrayList<Doublet<String, SQLType>>();
        l.add(new Doublet<String, SQLType>(a.nom, a.monType));

        for (int i = 0; i < a.fils.size(); i++) {
            List<Doublet<String, SQLType>> lRec = getMapNameSQLType(a.fils.get(i));
            l.addAll(lRec);
        }
        return l;
    }

    /**
     * retourne la commande sql creant le schema en etoile de l'arbre
     * TFnom pour la table des faits
     * TOnom pour les autres tables
     * @return la commande sql permettant de creer un schema en etoile correspondant
     */
    public String toSQLStarSchema() {
        List<List<Doublet<String, SQLType>>> l = new ArrayList<List<Doublet<String, SQLType>>>();

        l.add(new ArrayList<Doublet<String, SQLType>>());
        l.get(0).add(new Doublet<String, SQLType>(nom, monType));

        //construction de la TF index 0
        OlapArbre f;
        for (int i = 0; i < fils.size(); i++) {
            f = fils.get(i);
            l.get(0).add(new Doublet<String, SQLType>(f.nom, f.monType));
        }

        //construction des autres tables index 1...
        for (int i = 0; i < fils.size(); i++) {
            l.add(getMapNameSQLType(fils.get(i)));
        }

        //Parcours de l'arbre termin�
        //l.get(0) => table des faits
        //l.get(0).(n0, t0) => objet de l'analyse
        //l.get(1...) => autres tables
        //l.get(1...).get(0) => nom table et clef primaire
        //
        // g�n�ration de la requete


        List<Doublet<String, SQLType>> l2 = l.get(0);
        Doublet<String, SQLType> d;
        String tf = "TF" + l2.get(0).getFirst();
        String alterForeign = "";//= "ALTER TABLE " + tf + " ADD FOREIGN KEY (";
        String req = "CREATE TABLE " + tf + " (";

        for (int i = 1; i < l.get(0).size(); i++) {
            d = l2.get(i);
            req += d.getFirst() + " " + d.getSecond().getSQLType() + " NOT NULL, ";
        }
        req = req.substring(0, req.length() - 2);
        req += ");___";

        //g�n�ration des autres tables
        for (int i = 1; i < l.size(); i++) {
            l2 = l.get(i);
            if (l2.size() > 1) { // si table a creer
                for (int j = 0; j < l2.size(); j++) {
                    d = l2.get(j);
                    if (j == 0) { // cle primaire
                        //generation des foreign
                        //alterForeign += d.getFirst() + " REFERENCES TO" + d.getFirst() + "(" + d.getFirst() + ")" + " ,";
                        alterForeign += "ALTER TABLE " + tf + " ADD FOREIGN KEY (" + d.getFirst()
                                + ") REFERENCES TO" + d.getFirst() + " (" + d.getFirst() + ");___";


                        req += "CREATE TABLE TO" + d.getFirst() + " (";
                        req += d.getFirst() + " " + d.getSecond().getSQLType() + " PRIMARY KEY";
                    } else {
                        req += ", " + d.getFirst() + " " + d.getSecond().getSQLType() + " NOT NULL";
                    }
                }
                req += ");___";
            }
        }

        alterForeign = alterForeign.substring(0, alterForeign.length() - 1);

        req += alterForeign;
        /*
        try {
            String code = toB64String(this);
            String olapS = "_CREATE TABLE olapArbre (arbre TEXT NOT NULL);___"
                    + "INSERT INTO olapArbre (arbre) VALUES ('" + code + "');";
            System.out.println("INSERT INTO olapArbre (arbre) VALUES ('" + code + "');");
            req += olapS;


        } catch (IOException e) {
            e.printStackTrace();
        }
        */
        String olapS = "_CREATE TABLE olapArbre (src VARCHAR(50) NOT NULL, type INTEGER NOT NULL, dst VARCHAR(50) NOT NULL, typef INTEGER NOT NULL);___";
        req += olapS + generateInsert();
        System.out.println(req);
        return req;
    }
    
    private String generateInsert () {
    	String s = "";
    	for(OlapArbre a : fils) {
    		s += "INSERT INTO olapArbre (src, type, dst, typef) VALUES ('" + nom + "', " + monType.getTypeJSql() + ", '" + a.nom + "', " + a.monType.getTypeJSql() +");___";
    		s += a.generateInsert();
    	}
    	return s;
    }

    public boolean isRacine() {
        return pere == null;
    }

    public boolean hasFils() {
        return !fils.isEmpty();
    }

    public List<OlapArbre> getFils() {
        return fils;
    }

    public OlapArbre getPere() {
        return pere;
    }

    public String getNom() {
        return nom;
    }

    public int getId() {
        return id;
    }

    public SQLType getType() {
        return monType;
    }

    @Override
    public void addTreeModelListener(TreeModelListener l) {
        treeModListList.add(l);
    }

    @Override
    public OlapArbre getChild(Object parent, int index) {
        OlapArbre a = (OlapArbre) parent;
        return a.fils.get(index);
    }

    @Override
    public int getChildCount(Object parent) {
        OlapArbre a = (OlapArbre) parent;
        return a.fils.size();
    }

    @Override
    public int getIndexOfChild(Object parent, Object child) {
        OlapArbre a = (OlapArbre) parent;
        if (a.fils.contains(child)) {
            return a.fils.indexOf(child);
        } else {
            return -1;
        }
    }

    @Override
    public Object getRoot() {
        if (this.pere == null) {
            return this;
        } else {
            return pere.getRoot();
        }
    }

    @Override
    public boolean isLeaf(Object node) {
        OlapArbre a = (OlapArbre) node;
        return a.fils.isEmpty();
    }

    @Override
    public void removeTreeModelListener(TreeModelListener l) {
        treeModListList.remove(l);
    }

    /**
     * Messaged when the user has altered the value for the item identified by path to newValue.
     * If newValue signifies a truly new value the model should post a treeNodesChanged event.
     */
    @Override
    public void valueForPathChanged(TreePath path, Object newValue) {
        OlapArbre a = (OlapArbre) path.getLastPathComponent();
        if (newValue instanceof String) {
            nom = (String) newValue;
            for (TreeModelListener t : treeModListList) {
                t.treeNodesChanged(new TreeModelEvent(a, path));
            }
        }
    }

    @Override
    public String toString() {
        return this.nom;
    }

    public OlapArbre getPereBase1() {
        if (this.getPere() == this.getRoot()) {
            return this;
        }
        return this.getPere().getPereBase1();
    }

    public OlapArbre getRacine() {
        return (OlapArbre) getRoot();
    }

    private void writeObject(java.io.ObjectOutputStream out) throws IOException {
        // appel des mécanismes de sérialisation par défaut
        //out.defaultWriteObject();
        writeArbre(out);
        out.writeObject(hashTable);
        out.write(cptInst);
        out.writeObject(treeModListList);
    }

    private void writeArbre(java.io.ObjectOutputStream out) throws IOException {
        out.write(nom.length());
        out.writeChars(nom);
        out.write(id);
        out.writeObject(monType);
        out.write(fils.size());
        for (OlapArbre a : fils) {
            a.writeArbre(out);
        }
    }

    @SuppressWarnings("unchecked")
    private void readObject(java.io.ObjectInputStream in) throws IOException, ClassNotFoundException {
        readArbre(in);
        hashTable = (Map<Integer, OlapArbre>) in.readObject();
        cptInst = in.read();
        treeModListList = (List<TreeModelListener>) in.readObject();
    }

    private OlapArbre readArbre(java.io.ObjectInputStream in) throws IOException, ClassNotFoundException {
        int n = in.readInt();
        char[] value = new char[n];
        for (int i = 0; i < n; i++) {
            value[i] = in.readChar();
        }
        nom = new String(value);
        id = in.read();
        monType = (SQLType) in.readObject();
        n = in.read();
        for (int i = 0; i < n; i++) {
            OlapArbre a = readArbre(in);
            this.ajouterFils(a);
        }
        return this;
    }

    /** Read the object from Base64 string. */
    public static Object fromB64String(String s) throws IOException, ClassNotFoundException {
        byte[] data = Base64.decode(s);
        ByteArrayInputStream bais = new ByteArrayInputStream(data);
        ObjectInputStream ois = new ObjectInputStream(bais);
        Object o = ois.readObject();
        ois.close();
        bais.close();
        return o;
    }

    /** Write the object to a Base64 string. */
    public static String toB64String(Serializable o) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        ObjectOutputStream oos = new ObjectOutputStream(baos);
        oos.writeObject(o);
        oos.flush();
        oos.close();
        return new String(Base64.encode(baos.toByteArray()));
    }

    public void initHashTable() {
        hashTable.clear();
        cptInst = 0;
        addHashTable();
    }

    private void addHashTable() {
        hashTable.put(cptInst, this);
        cptInst++;
        for (OlapArbre a : fils) {
            a.addHashTable();
        }

    }
}
